/**
 *  @file    tcp_socket_tcp_server_test.h
 *
 *  @author  Tobias Anker <tobias.anker@kitsunemimi.moe>
 *
 *  @copyright MIT License
 */

#ifndef TCPSOCKET_TCPSERVER_TEST_H
#define TCPSOCKET_TCPSERVER_TEST_H

#include <libKitsunemimiCommon/test.h>

namespace Kitsunemimi
{
class DataBuffer;
namespace Network
{
class TcpServer;
class TcpSocket;
class MessageRingBuffer;
class AbstractSocket;

class TcpSocket_TcpServer_Test
        : public Kitsunemimi::Test
{
public:
    TcpSocket_TcpServer_Test();

private:
    void initTestCase();
    void checkConnectionInit();
    void checkLittleDataTransfer();
    void checkBigDataTransfer();
    void cleanupTestCase();

    TcpServer* m_server = nullptr;
    TcpSocket* m_socketClientSide = nullptr;
    TcpSocket* m_socketServerSide = nullptr;
    DataBuffer* m_buffer = nullptr;
};

} // namespace Network
} // namespace Kitsunemimi

#endif // TCPSOCKET_TCPSERVER_TEST_H
