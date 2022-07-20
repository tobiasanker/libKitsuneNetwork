﻿/**
 *  @file    unix_domain_socket_unix_domain_server_test.cpp
 *
 *  @author  Tobias Anker <tobias.anker@kitsunemimi.moe>
 *
 *  @copyright MIT License
 */

#include "unix_domain_test.h"
#include <libKitsunemimiCommon/buffer/ring_buffer.h>

#include <libKitsunemimiNetwork/unix/unix_domain_socket.h>
#include <libKitsunemimiNetwork/unix/unix_domain_server.h>
#include <libKitsunemimiNetwork/net_socket.h>
#include <libKitsunemimiNetwork/net_server.h>

namespace Kitsunemimi
{
namespace Network
{

/**
 * processMessageUnixDomain-callback
 */
uint64_t processMessageUnixDomain(void* target,
                                  Kitsunemimi::RingBuffer* recvBuffer,
                                  NetSocket<UnixDomainSocket>*)
{
    UnixDomain_Test* targetTest = static_cast<UnixDomain_Test*>(target);
    const uint8_t* dataPointer = getDataPointer_RingBuffer(*recvBuffer, recvBuffer->usedSize);
    if(dataPointer == nullptr) {
        return 0;
    }

    addData_DataBuffer(*targetTest->m_buffer, dataPointer, recvBuffer->usedSize);
    return recvBuffer->usedSize;
}

/**
 * processConnectionUnixDomain-callback
 */
void processConnectionUnixDomain(void* target,
                                 NetSocket<UnixDomainSocket>* socket)
{
    UnixDomain_Test* targetTest = static_cast<UnixDomain_Test*>(target);
    targetTest->m_socketServerSide = socket;
    socket->setMessageCallback(target, &processMessageUnixDomain);
    socket->startThread();
}


UnixDomain_Test::UnixDomain_Test()
    : Kitsunemimi::MemoryLeakTestHelpter("UnixDomain_Test")
{
    ErrorContainer* error = nullptr;

    // init for one-time-allocations
    error = new ErrorContainer();

    UnixDomainServer udsServer2("/tmp/sock.uds");
    udsServer2.initServer(*error);
    m_server = new NetServer<UnixDomainServer>(std::move(udsServer2),
                                               this,
                                               &processConnectionUnixDomain,
                                               "UnixDomain_Test");
    m_server->scheduleThreadForDeletion();
    sleep(2);

    // create new test-server
    REINIT_TEST();
    m_buffer = new DataBuffer(1000);
    error = new ErrorContainer();
    UnixDomainServer udsServer("/tmp/sock.uds");
    udsServer.initServer(*error);
    m_server = new NetServer<UnixDomainServer>(std::move(udsServer),
                                               this,
                                               &processConnectionUnixDomain,
                                               "UnixDomain_Test");
    m_server->startThread();

        // test client create and delete
        UnixDomainSocket udsSocket("/tmp/sock.uds");
        udsSocket.initClientSide(*error);
        m_socketClientSide = new NetSocket<UnixDomainSocket>(std::move(udsSocket),
                                                             "UnixDomain_Test_client");
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
