/**
 *  @file    tcpClient.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  MIT License
 */

#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <cinttypes>
#include <unistd.h>
#include <string>
#include <vector>

#include <threading/commonThread.h>
#include <buffering/commonDataBuffer.h>

#include <cleanupThread.h>
#include <messageringbuffer.h>

namespace Kitsune
{
namespace Network
{
class NetworkTrigger;

class TcpClient : public Kitsune::CommonThread
{
public:
    TcpClient(const std::string address,
              const uint16_t port);
    TcpClient(const int clientFd,
              struct sockaddr_in client);
    ~TcpClient();

    static Kitsune::Network::CleanupThread* m_cleanup;

    bool addNetworkTrigger(NetworkTrigger* trigger);
    bool removeNetworkTrigger(const uint32_t index);
    void clearNetworkTrigger();

    bool sendMessage(const std::string &message);
    bool sendMessage(const uint8_t *message, const uint32_t numberOfBytes);

    bool closeSocket();

protected:
    void run();

private:
    bool m_clientSide = false;
    std::string m_address = "";
    uint16_t m_port = 0;

    MessageRingBuffer m_recvBuffer;

    int m_clientSocket = 0;
    sockaddr_in m_client;

    std::vector<NetworkTrigger*> m_trigger;

    bool initClientSide();
    bool waitForMessage();
};

}
}

#endif // TCPCLIENT_H
