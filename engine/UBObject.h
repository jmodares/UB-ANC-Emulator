#ifndef UBOBJECT_H
#define UBOBJECT_H

#include <QThread>
#include <QGeoCoordinate>

#include "ns3/ipv4-address.h"

class Vehicle;

class UBObject;

class QTcpSocket;
class QTcpServer;
class QHash<class K, class V>;

namespace ns3 {
class Node;
class Socket;
class Vector3D;
class MobilityModel;
class InetSocketAddress;
}

typedef QHash<quint8, UBObject*> OHash;
typedef QHash<quint8, ns3::Ipv4Address> AHash;

class UBObject : public QThread
{
    Q_OBJECT
public:
    explicit UBObject(quint32 rng, ns3::Node* node, OHash *lnks, AHash *adrs, QThread *parent = nullptr);

public:
    quint8 getID(void) {return m_id;}
    Vehicle* getMAV(void) {return m_mav;}

public slots:
    void setID(quint8 id);
    void setRange(quint32 rng) {m_range = rng;}
    void setMAV(Vehicle* mav);

    void sendData(QByteArray data);

protected slots:
    void coordinateChangeEvent(QGeoCoordinate pos);
    void coordinateChangeEventNS3(ns3::MobilityModel* mm, ns3::Vector3D* pos);

    void dataReadyEvent();
    void dataReadyEventNS3(ns3::Socket* socket, void* packet, ns3::InetSocketAddress* remote);

    void newConnectionEvent();
    void disconnectedEvent();

protected:
    quint8 m_id;
    quint32 m_range;

    Vehicle* m_mav;
    ns3::Node* m_node;

    OHash* m_lnks;
    AHash* m_adrs;

    QByteArray* m_buffer;

    QTcpSocket* m_socket;
    QTcpServer* m_server;
};

#endif // UBOBJECT_H
