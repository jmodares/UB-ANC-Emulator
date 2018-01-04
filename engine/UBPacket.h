#ifndef UBPACKET_H
#define UBPACKET_H

#include <QObject>

class UBPacket
{
public:
    explicit UBPacket();

public slots:
    void setSrcID(quint8 srcID) {m_srcID = srcID;}
    void setDesID(quint8 desID) {m_desID = desID;}
    void setPayload(const QByteArray& payload) {m_payload = payload;}

    quint8 getSrcID(void) {return m_srcID;}
    quint8 getDesID(void) {return m_desID;}
    QByteArray getPayload(void) {return m_payload;}

    QByteArray packetize(void);
    void depacketize(const QByteArray &packet);

protected:
    quint8 m_srcID;
    quint8 m_desID;

    QByteArray m_payload;
};

#endif // UBPACKET_H
