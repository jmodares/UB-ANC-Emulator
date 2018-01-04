#ifndef UBENGINE_H
#define UBENGINE_H

#include <QThread>

#include "ns3/socket.h"

class Vehicle;
class UBObject;
class UBControl;

class QHash<class K, class V>;

namespace ns3 {
class NodeContainer;
}

typedef QHash<quint8, UBObject*> OHash;
typedef QHash<quint8, ns3::Ipv4Address> AHash;
typedef QHash<ns3::Node*, UBObject*> NHash;

class UBEngine : public QThread
{
    Q_OBJECT
public:
    explicit UBEngine(QThread *parent = nullptr);

public slots:
    void startEngine();

protected slots:
    void focusChangedEvent();
    void lastWindowClosedEvent();
    void vehicleAddedEvent(Vehicle* mav);
    void vehicleRemovedEvent(Vehicle* mav);

    void packetReceivedEvent(ns3::Ptr<ns3::Socket> socket);

protected:
    void run() override;

    void setupNetwork(const ns3::NodeContainer& nodes, const std::string& routing, bool tracing);

protected:
    quint32 m_range;

    UBControl* m_cntl;

    OHash* m_objs;
    OHash* m_lnks;
    AHash* m_adrs;
    NHash* m_nods;
};

#endif // UBENGINE_H
