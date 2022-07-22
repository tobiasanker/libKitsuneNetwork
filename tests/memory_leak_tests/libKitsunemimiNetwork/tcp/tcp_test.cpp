/**
 *  @file    tcp_socket_tcp_server_test.cpp
 *
 *  @author  Tobias Anker <tobias.anker@kitsunemimi.moe>
 *
 *  @copyright MIT License
 */

#include "tcp_test.h"
#include <libKitsunemimiCommon/buffer/ring_buffer.h>

#include <libKitsunemimiCommon/threading/thread_handler.h>
#include <libKitsunemimiNetwork/net_socket.h>
#include <libKitsunemimiNetwork/net_server.h>

namespace Kitsunemimi
{
namespace Network
{

/**
 * processMessageTcp-callback
 */
uint64_t processMessageTcp(void* target,
                           Kitsunemimi::RingBuffer* recvBuffer,
                           NetSocket<TcpSocket>*)
{
    Tcp_Test* targetTest = static_cast<Tcp_Test*>(target);
    const uint8_t* dataPointer = getDataPointer_RingBuffer(*recvBuffer, recvBuffer->usedSize);
    if(dataPointer == nullptr) {
        return 0;
    }

    addData_DataBuffer(*targetTest->m_buffer, dataPointer, recvBuffer->usedSize);
    return recvBuffer->usedSize;
}

/**
 * processConnectionTcp-callback
 */
void processConnectionTcp(void* target,
                          NetSocket<TcpSocket>* socket)
{
    Tcp_Test* targetTest = static_cast<Tcp_Test*>(target);
    targetTest->m_socketServerSide = socket;
    socket->setMessageCallback(target, &processMessageTcp);
    socket->startThread();
}


Tcp_Test::Tcp_Test()
    : Kitsunemimi::MemoryLeakTestHelpter("Tcp_Test")
{
    ErrorContainer* error = new ErrorContainer();

    // init for one-time-allocations
    TcpServer tcpServer2(12345);
    tcpServer2.initServer(*error);
    m_server = new NetServer<TcpServer>(std::move(tcpServer2),
                                        this,
                                        &processConnectionTcp,
                                        "Tcp_Test");

    m_server->scheduleThreadForDeletion();
    sleep(2);

    // create new test-server
    REINIT_TEST();
    m_buffer = new DataBuffer(1000);
    error = new ErrorContainer();
    TcpServer tcpServer(12345);
    tcpServer.initServer(*error);
    m_server = new NetServer<TcpServer>(std::move(tcpServer),
                                        this,
                                        &processConnectionTcp,
                                        "Tcp_Test");
    m_server->startThread();

        // test client create and delete
        TcpSocket tcpSocket("127.0.0.1", 12345);
        m_socketClientSide = new NetSocket<TcpSocket>(std::move(tcpSocket),
                                                      "Tcp_Test_client");
        m_socketClientSide->initConnection(*error);

        sleep(2);

            // send messages
            std::string sendMessage("poipoipoi");
            m_socketClientSide->sendMessage(sendMessage, *error);
            usleep(100000);

            std::string sendMessage2 = "poi";
            m_socketClientSide->sendMessage(sendMessage2, *error);
            for(uint32_t i = 0; i < 99999; i++) {
                m_socketClientSide->sendMessage(sendMessage2, *error);
            }

        m_socketServerSide->closeSocket();
        m_socketServerSide->scheduleThreadForDeletion();
        m_socketClientSide->closeSocket();
        m_socketClientSide->scheduleThreadForDeletion();
        sleep(2);

    // clear test-server
    m_server->closeServer();
    m_server->scheduleThreadForDeletion();
    sleep(2);
    delete m_buffer;
    delete error;
    CHECK_MEMORY();
}

} // namespace Network
} // namespace Kitsunemimi