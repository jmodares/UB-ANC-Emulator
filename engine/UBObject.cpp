#include "UBObject.h"
#include "UBPacket.h"
#include "UBConfig.h"

#include <QHash>
#include <QTcpSocket>
#include <QTcpServer>

#include "Vehicle.h"

#include "ns3/node.h"
#include "ns3/socket.h"
#include "ns3/simulator.h"
#include "ns3/mobility-model.h"
#include "ns3/geographic-positions.h"

UBObject::UBObject(quint32 rng, ns3::Node* node, OHash* lnks, AHash* adrs, QThread *parent) : QThread(parent),
    m_id(0),
    m_range(rng),
    m_mav(nullptr),
    m_node(node),
    m_lnks(lnks),
    m_adrs(adrs),
    m_socket(nullptr)
{
    m_buffer = new QByteArray;
    m_server = new QTcpServer;

    this->moveToThread(this);
    m_server->moveToThread(this);

    connect(m_server, SIGNAL(newConnection()), this, SLOT(newConnectionEvent()));

    start();
}

void UBObject::setID(quint8 id) {
    if (m_id) {
        m_server->close();
    }

    m_id = id;

    if (m_id) {
        m_server->listen(QHostAddress::Any, 10 * m_id + NET_PORT);
    }
}

void UBObject::setMAV(Vehicle *mav) {
    if (m_mav) {
        disconnect(m_mav, SIGNAL(coordinateChanged(QGeoCoordinate)), this, SLOT(coordinateChangeEvent(QGeoCoordinate)));
    }

    m_mav = mav;

    if (m_mav) {
        connect(m_mav, SIGNAL(coordinateChanged(QGeoCoordinate)), this, SLOT(coordinateChangeEvent(QGeoCoordinate)));
    }
}

void UBObject::newConnectionEvent() {
    if (m_socket) {
        m_socket->disconnectFromHost();
    }

    m_socket = m_server->nextPendingConnection();
    m_socket->moveToThread(this);

    connect(m_socket, SIGNAL(readyRead()), this, SLOT(dataReadyEvent()));
    connect(m_socket, SIGNAL(disconnected()), this, SLOT(disconnectedEvent()));

    qInfo() << "New agent connected on port: " << m_socket->localPort();
}

void UBObject::disconnectedEvent() {
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    if (socket) {
        socket->deleteLater();

        qInfo() << "An agent disconnected on port: " << socket->localPort();
    }

    if (m_socket == socket) {
        m_socket = nullptr;
    }
}

void UBObject::coordinateChangeEvent(QGeoCoordinate pos) {
    if (!m_node) {
        return;
    }

    ns3::Ptr<ns3::MobilityModel> mm = m_node->GetObject<ns3::MobilityModel>();
    if (!mm) {
        return;
    }

    ns3::Vector3D _pos = ns3::GeographicPositions::GeographicToCartesianCoordinates(pos.latitude(), pos.longitude(), pos.altitude(), ns3::GeographicPositions::WGS84);
    ns3::Vector3D* _p = new ns3::Vector3D(_pos.x, _pos.y, _pos.y);

//    ns3::Simulator::ScheduleWithContext(m_node->GetId(), ns3::Time(0), &ns3::MobilityModel::SetPosition, ns3::GetPointer(mm), _pos);
    ns3::Simulator::ScheduleWithContext(m_node->GetId(), ns3::Seconds(0), &UBObject::coordinateChangeEventNS3, this, ns3::GetPointer(mm), _p);
}

void UBObject::coordinateChangeEventNS3(ns3::MobilityModel* mm, ns3::Vector3D* pos) {
    mm->SetPosition(*pos);

    delete pos;
}

void UBObject::dataReadyEvent() {
    if (!m_socket) {
        return;
    }

    m_buffer->append(m_socket->readAll());

    if (!m_node || !m_lnks || !m_adrs || !m_lnks->value(m_id, nullptr)) {
        m_buffer->clear();

        return;
    }

    ns3::Ptr<ns3::Socket> socket = m_node->GetObject<ns3::Socket>();
    if (!socket) {
        return;
    }

    while (true) {
        int bytes = m_buffer->indexOf(PACKET_END);
        if (bytes == -1) {
            return;
        }

        QByteArray* packet = new QByteArray(m_buffer->left(bytes));
        m_buffer->remove(0, bytes + qstrlen(PACKET_END));

        UBPacket pkt;
        pkt.depacketize(*packet);
        packet->append(PACKET_END);

        UBObject* dest = m_lnks->value(pkt.getDesID(), nullptr);
        if (!dest) {
            continue;
        }

        if (m_range) {
            if (!m_mav || !dest->getMAV() || m_mav->coordinate().distanceTo(dest->getMAV()->coordinate()) > m_range) {
                continue;
            }

            QMetaObject::invokeMethod(dest, "sendData", Qt::QueuedConnection, Q_ARG(QByteArray, *packet));
        } else {
//            InetSocketAddress remote = InetSocketAddress(Ipv4Address::GetBroadcast(), PXY_PORT);
//            InetSocketAddress remote = InetSocketAddress(Ipv4Address(tr("10.1.1.%1").arg(packet.getDesID()).toStdString().c_str()), PXY_PORT);
            ns3::InetSocketAddress* remote = new ns3::InetSocketAddress(m_adrs->value(pkt.getDesID()), PXY_PORT);

            ns3::Simulator::ScheduleWithContext(m_node->GetId(), ns3::Seconds(0), &UBObject::dataReadyEventNS3, this, ns3::GetPointer(socket), (void*)packet, remote);
        }
    }
}

void UBObject::dataReadyEventNS3(ns3::Socket* socket, void* packet, ns3::InetSocketAddress* remote) {
    QByteArray* pkt = (QByteArray*)packet;
    socket->SendTo((uint8_t*)pkt->data(), pkt->size(), 0, *remote);

    delete pkt;
    delete remote;
}

void UBObject::sendData(QByteArray data) {
    if (!m_socket) {
        return;
    }

    m_socket->write(data);
}
