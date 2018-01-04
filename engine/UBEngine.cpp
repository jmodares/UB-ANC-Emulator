#include "UBEngine.h"
#include "UBObject.h"
#include "UBControl.h"
#include "UBCmdLn.h"

#include "UBConfig.h"

#include <QDir>
#include <QHash>
#include <QMenuBar>
#include <QMainWindow>

#include "TCPLink.h"
#include "QGCApplication.h"

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"
#include "ns3/olsr-module.h"
#include "ns3/aodv-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("UBEngine");

UBEngine::UBEngine(QThread *parent) : QThread(parent),
    m_range(0),
    m_cntl(nullptr)
{
    m_objs = new OHash;
    m_lnks = new OHash;
    m_adrs = new AHash;
    m_nods = new NHash;

    connect(QApplication::instance(), SIGNAL(focusChanged(QWidget*,QWidget*)), this, SLOT(focusChangedEvent()));
    connect(QApplication::instance(), SIGNAL(lastWindowClosed()), this, SLOT(lastWindowClosedEvent()));

    connect(qgcApp()->toolbox()->multiVehicleManager(), SIGNAL(vehicleAdded(Vehicle*)), this, SLOT(vehicleAddedEvent(Vehicle*)));
    connect(qgcApp()->toolbox()->multiVehicleManager(), SIGNAL(vehicleRemoved(Vehicle*)), this, SLOT(vehicleRemovedEvent(Vehicle*)));

    startEngine();
}

void UBEngine::startEngine() {
    GlobalValue::Bind("SimulatorImplementationType", StringValue("ns3::RealtimeSimulatorImpl"));
//    GlobalValue::Bind("ChecksumEnabled", BooleanValue(false));

//    Config::SetDefault("ns3::YansWifiChannel::PropagationDelayModel", StringValue("ns3::ConstantSpeedPropagationDelayModel"));
//    Config::SetDefault("ns3::YansWifiChannel::PropagationLossModel", StringValue("ns3::FriisPropagationLossModel"));

    // disable fragmentation for frames below 2200 bytes
    Config::SetDefault("ns3::WifiRemoteStationManager::FragmentationThreshold", StringValue("2200"));
    // turn off RTS/CTS for frames below 2200 bytes
    Config::SetDefault("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue("2200"));
    // Fix non-unicast data rate to be the same as that of unicast
//    Config::SetDefault("ns3::WifiRemoteStationManager::NonUnicastMode", StringValue(phyMode));

    int argc = 0;
    QStringList args = QApplication::arguments();
    char** argv = new char*[args.size() + 1];
    for (QString str : args) {
        QByteArray arg = str.toLocal8Bit();
        argv[argc] = new char[arg.size() + 1];
        qstrcpy(argv[argc], arg.constData());
        argc++;
    }
    argv[argc] = nullptr;

//    std::string phyMode("DsssRate1Mbps");
    std::string routing("olsr");
//    bool verbose = false;
    bool tracing = false;

    UBCmdLn cmd;

    cmd.Usage("Network Related Options...");

    cmd.AddValue("range", "Simple Network Simulation Range, or 0 for switching to ns-3 Network Simulation", m_range);
//    cmd.AddValue("phyMode", "Wifi Phy mode", phyMode);
    cmd.AddValue("routing", "Routing: olsr/aodv", routing);
//    cmd.AddValue("verbose", "turn on all WifiNetDevice log components", verbose);
    cmd.AddValue("tracing", "turn on ascii and pcap tracing", tracing);

    cmd.Parse(argc, argv);

    for (int i = 0; i < argc; i++) {
        delete argv[i];
    }
    delete argv;

    NodeContainer nodes;

    QDir objs(OBJ_DIR);
    objs.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);

    for (QString name : objs.entryList()) {
        name = name.mid(name.indexOf(MAV_DIR) + qstrlen(MAV_DIR));

        quint8 id = name.toUInt();
        Ptr<Node> node = CreateObject<Node>();
        UBObject* obj = new UBObject(m_range, GetPointer(node), m_lnks, m_adrs);
        QMetaObject::invokeMethod(obj, "setID", Qt::QueuedConnection, Q_ARG(quint8, id));

//        UBAgent* agent = new UBAgent(id); 

        m_objs->insert(id, obj);
        m_lnks->insert(id, obj);
        m_nods->insert(GetPointer(node), obj);

        nodes.Add(node);

//        MockLink::startAPMArduCopterMockLink(false);

        quint32 port = 10 * id + STL_PORT + 2;
        TCPConfiguration* tcpConfig = new TCPConfiguration(tr("TCP Port %1").arg(port));

        tcpConfig->setAddress(QHostAddress::LocalHost);
        tcpConfig->setPort(port);
        tcpConfig->setDynamic();
        tcpConfig->setAutoConnect();

        LinkManager* linkManager = qgcApp()->toolbox()->linkManager();
        linkManager->addConfiguration(tcpConfig);
        linkManager->linkConfigurationsChanged();
    }

    setupNetwork(nodes, routing, tracing);
}

void UBEngine::setupNetwork(const NodeContainer& nodes, const std::string& routing, bool tracing) {
    // The below set of helpers will help us to put together the wifi NICs we want
    WifiHelper wifi;
//    if (verbose) {
//        wifi.EnableLogComponents();  // Turn on all Wifi logging
//    }

    YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default();
    // set it to zero; otherwise, gain will be added
//    wifiPhy.Set("RxGain", DoubleValue(-10));
    // ns-3 supports RadioTap and Prism tracing extensions for 802.11b
    wifiPhy.SetPcapDataLinkType(YansWifiPhyHelper::DLT_IEEE802_11_RADIO);

//    YansWifiChannelHelper wifiChannel;
//    wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
//    wifiChannel.AddPropagationLoss("ns3::FriisPropagationLossModel");
//    wifiPhy.SetChannel(wifiChannel.Create());
    wifiPhy.SetChannel(CreateObject<YansWifiChannel>());

    // Add an upper mac and disable rate control
    WifiMacHelper wifiMac;
    wifi.SetStandard(WIFI_PHY_STANDARD_80211b);
//    wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager", "DataMode", StringValue(phyMode), "ControlMode", StringValue(phyMode));
    wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager");
    // Set it to adhoc mode
    wifiMac.SetType("ns3::AdhocWifiMac");
    NetDeviceContainer devices = wifi.Install(wifiPhy, wifiMac, nodes);

    MobilityHelper mobility;
    Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator>();
    for (quint32 i = 0; i < nodes.GetN(); ++i) {
        positionAlloc->Add(Vector(0.0, 0.0, 0.0));
    }
    mobility.SetPositionAllocator(positionAlloc);
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(nodes);

    // Enable OLSR
    OlsrHelper olsr;
    AodvHelper aodv;
    Ipv4StaticRoutingHelper staticRouting;

    Ipv4ListRoutingHelper list;
    list.Add(staticRouting, 0);
    if (routing == "olsr") {
        list.Add(olsr, 10);
    } else {
        list.Add(aodv, 10);
    }

    InternetStackHelper internet;
    internet.SetRoutingHelper(list); // has effect on the next Install()
    internet.Install(nodes);

    Ipv4AddressHelper ipv4;
    NS_LOG_INFO("Assign IP Addresses.");
    ipv4.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer i = ipv4.Assign(devices);

    for (Ipv4InterfaceContainer::Iterator it = i.Begin(); it != i.End(); ++it) {
        Ptr<Node> node = it->first->GetNetDevice(it->second)->GetNode();
        UBObject* obj = m_nods->value(GetPointer(node));
        Ipv4Address addr = it->first->GetAddress(it->second, 0).GetLocal();

        m_adrs->insert(obj->getID(), addr);
    }

    InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(), PXY_PORT);
    for (NodeContainer::Iterator it = nodes.Begin(); it != nodes.End(); ++it) {
        Ptr<Socket> socket = Socket::CreateSocket(*it, UdpSocketFactory::GetTypeId());
        socket->SetAllowBroadcast(true);
        socket->Bind(local);
        socket->SetRecvCallback(MakeCallback(&UBEngine::packetReceivedEvent, this));

        (*it)->AggregateObject(socket);
    }

    if (tracing) {
        AsciiTraceHelper ascii;
        wifiPhy.EnableAsciiAll(ascii.CreateFileStream("wifi-adhoc.tr"));
        wifiPhy.EnablePcap("wifi-adhoc", devices);
        // Trace routing tables
        if (routing == "olsr") {
            Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper>("olsr.routes", std::ios::out);
            olsr.PrintRoutingTableAllEvery(Seconds(SAVE_RATE), routingStream);
            Ptr<OutputStreamWrapper> neighborStream = Create<OutputStreamWrapper>("olsr.neighbors", std::ios::out);
            olsr.PrintNeighborCacheAllEvery(Seconds(SAVE_RATE), neighborStream);

            // To do-- enable an IP-level trace that shows forwarding events only
        } else {
            Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper>("aodv.routes", std::ios::out);
            aodv.PrintRoutingTableAllEvery(Seconds(SAVE_RATE), routingStream);
            Ptr<OutputStreamWrapper> neighborStream = Create<OutputStreamWrapper>("aodv.neighbors", std::ios::out);
            aodv.PrintNeighborCacheAllEvery(Seconds(SAVE_RATE), neighborStream);
        }
    }

    // Give OLSR time to converge-- 30 seconds perhaps

    start();
}

void UBEngine::run() {
    Simulator::Run();
    Simulator::Destroy();
}

void UBEngine::focusChangedEvent() {
    if (m_cntl) {
        return;
    }

    QMainWindow* mainWnd = nullptr;
    for (QWidget* w : QApplication::allWidgets()) {
        mainWnd = qobject_cast<QMainWindow*>(w);
        if (mainWnd) {
            break;
        }
    }

    if (!mainWnd) {
        return;
    }

    QAction* act = new QAction(tr("&Network"), mainWnd);
    act->setCheckable(true);
    act->setShortcuts(QKeySequence::Refresh);
    act->setStatusTip(tr("Open Network Simulation Setup"));

    for (QMenu* m : mainWnd->menuBar()->findChildren<QMenu*>()) {
        if (m->title() == tr("Widgets")) {
            m->addSeparator();
            m->addAction(act);

            break;
        }
    }

//    QMenu* menu = mainWnd->menuBar()->addMenu(tr("&Simulation"));
//    menu->addAction(act);

    m_cntl = new UBControl(m_range, m_objs, m_lnks, tr("UB-ANC Emulator Setup"), act, mainWnd);

    connect(act, SIGNAL(triggered(bool)), m_cntl, SLOT(setVisible(bool)));

    act->trigger();
}

void UBEngine::lastWindowClosedEvent() {
    Simulator::Stop(Seconds(0));
}

void UBEngine::vehicleAddedEvent(Vehicle* mav) {
    if (!mav) {
        return;
    }
    quint8 id = mav->id();

    UBObject* obj = m_objs->value(mav->id(), nullptr);
    if (!obj) {
        return;
    }

    QMetaObject::invokeMethod(obj, "setMAV", Qt::QueuedConnection, Q_ARG(Vehicle*, mav));
    qInfo() << "New MAV connected with ID: " << id;
}

void UBEngine::vehicleRemovedEvent(Vehicle* mav) {
    if (!mav) {
        return;
    }
    quint8 id = mav->id();

    UBObject* obj = m_objs->value(mav->id(), nullptr);
    if (!obj) {
        return;
    }

    QMetaObject::invokeMethod(obj, "setMAV", Qt::QueuedConnection, Q_ARG(Vehicle*, nullptr));
    qInfo() << "MAV disconnected with ID: " << id;
}

void UBEngine::packetReceivedEvent(Ptr<Socket> socket) {
    NS_LOG_FUNCTION(this << socket);

    Ptr<Node> node = socket->GetNode();
    if (!node) {
        return;
    }

    UBObject* obj = m_nods->value(GetPointer(node), nullptr);
    if (!obj) {
        return;
    }

    Ptr<Packet> packet;
    Address from;
    while ((packet = socket->RecvFrom(from))) {
        if (InetSocketAddress::IsMatchingType (from)) {
            NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s client received " << packet->GetSize () << " bytes from " <<
                         InetSocketAddress::ConvertFrom (from).GetIpv4 () << " port " <<
                         InetSocketAddress::ConvertFrom (from).GetPort ());
        } else if (Inet6SocketAddress::IsMatchingType (from)) {
            NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s client received " << packet->GetSize () << " bytes from " <<
                         Inet6SocketAddress::ConvertFrom (from).GetIpv6 () << " port " <<
                         Inet6SocketAddress::ConvertFrom (from).GetPort ());
        }

        QByteArray pkt = QByteArray(packet->GetSize(), 0);
        packet->CopyData((uint8_t*)pkt.data(), packet->GetSize());
        QMetaObject::invokeMethod(obj, "sendData", Qt::QueuedConnection, Q_ARG(QByteArray, pkt));
    }
}
