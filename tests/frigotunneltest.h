#ifndef FRIGOTUNNELTEST_H
#define FRIGOTUNNELTEST_H

#include <QObject>

class FrigoTunnelTest : public QObject
{
    Q_OBJECT
public:
    explicit FrigoTunnelTest(QObject *parent = 0);
    ~FrigoTunnelTest();

private slots:
    void packetPropagation();
    void udpReception();
    void tcpReception();
};

#endif // FRIGOTUNNELTEST_H
