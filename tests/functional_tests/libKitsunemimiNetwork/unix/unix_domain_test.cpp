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

namespace Kitsunemimi
{
namespace Network
{

/**
 * processMessageUnixDomain-callback
 */
uint64_t processMessageUnixDomain(void* target,
                                  Kitsunemimi::RingBuffer* recvBuffer,
                                  AbstractSocket*)
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
                                 AbstractSocket* socket)
{
    UnixDomain_Test* targetTest = static_cast<UnixDomain_Test*>(target);
    targetTest->m_socketServerSide = static_cast<UnixDomainSocket*>(socket);
    socket->setMessageCallback(target, &processMessageUnixDomain);
    socket->startThread();
}


UnixDomain_Test::UnixDomain_Test() :
    Kitsunemimi::CompareTestHelper("UnixDomain_Test")
{
    initTestCase();
    checkConnectionInit();
    checkLittleDataTransfer();
    checkBigDataTransfer();
    cleanupTestCase();
}

/**
 * initTestCase
 */
void
UnixDomain_Test::initTestCase()
{
    m_buffer = new DataBuffer(1000);
    m_server = new UnixDomainServer(this, &processConnectionUnixDomain, "UnixDomain_Test");
}

/**
 * checkConnectionInit
 */
void
UnixDomain_Test::checkConnectionInit()
{
    ErrorContainer error;
    // check too long path
    TEST_EQUAL(m_server->initServer("/tmp/sock.uds11111111111111111111111"
                                    "111111111111111111111111111111111111"
                                    "111111111111111111111111111111111111"
                                    "111111111111111111111111111111111111"
                                    "111111111111111111111111111111111111",
                                    error), false);
    // init server
    TEST_EQUAL(m_server->getType(), AbstractServer::UNIX_SERVER);
    TEST_EQUAL(m_server->initServer("/tmp/sock.uds", error), true);
    TEST_EQUAL(m_server->startThread(), true);

    usleep(100000);

    // check too long path
    UnixDomainSocket failSocket("/tmp/sock.uds11111111111111111111111"
                                "111111111111111111111111111111111111"
                                "111111111111111111111111111111111111"
                                "111111111111111111111111111111111111"
                                "111111111111111111111111111111111111",
                                "fail1");
    TEST_EQUAL(failSocket.initClientSide(error), false);

    // init client
    m_socketClientSide = new UnixDomainSocket("/tmp/sock.uds", "UnixDomain_Test_client");
    TEST_EQUAL(m_socketClientSide->initClientSide(error), true);
    TEST_EQUAL(m_socketClientSide->initClientSide(error), true);
    TEST_EQUAL(m_socketClientSide->getType(), AbstractSocket::UNIX_SOCKET);

    usleep(100000);
}

/**
 * checkLittleDataTransfer
 */
void
UnixDomain_Test::checkLittleDataTransfer()
{
    usleep(100000);
    ErrorContainer error;

    std::string sendMessage("poipoipoi");
    TEST_EQUAL(m_socketClientSide->sendMessage(sendMessage, error), true);
    usleep(100000);
    TEST_EQUAL(m_buffer->usedBufferSize, 9);

    if(m_buffer->usedBufferSize == 9)
    {
        DataBuffer* buffer = m_buffer;
        uint64_t bufferSize = buffer->usedBufferSize;
        char recvMessage[bufferSize];
        memcpy(recvMessage, buffer->data, bufferSize);
        TEST_EQUAL(bufferSize, 9);
        TEST_EQUAL(recvMessage[2], sendMessage.at(2));
        reset_DataBuffer(*m_buffer, 1000);
    }
}

/**
 * checkBigDataTransfer
 */
void
UnixDomain_Test::checkBigDataTransfer()
{
    ErrorContainer error;

    std::string sendMessage = "poi";
    TEST_EQUAL(m_socketClientSide->sendMessage(sendMessage, error), true);
    for(uint32_t i = 0; i < 99999; i++) {
        m_socketClientSide->sendMessage(sendMessage, error);
    }

    usleep(10000);
    uint64_t totalIncom = m_buffer->usedBufferSize;
    DataBuffer* dataBuffer = m_buffer;
    TEST_EQUAL(totalIncom, 300000);
    TEST_EQUAL(dataBuffer->usedBufferSize, 300000);

    uint32_t numberOfPois = 0;
    for(uint32_t i = 0; i < 300000; i=i+3)
    {
        uint8_t* dataBufferData = static_cast<uint8_t*>(dataBuffer->data);
        if(dataBufferData[i] == 'p'
                && dataBufferData[i+1] == 'o'
                && dataBufferData[i+2] == 'i')
        {
            numberOfPois++;
        }
    }

    TEST_EQUAL(numberOfPois, 100000);
}

/**
 * cleanupTestCase
 */
void
UnixDomain_Test::cleanupTestCase()
{
    TEST_EQUAL(m_socketServerSide->closeSocket(), true);
    TEST_EQUAL(m_server->closeServer(), true);
    TEST_EQUAL(m_socketServerSide->scheduleThreadForDeletion(), true);
    TEST_EQUAL(m_server->scheduleThreadForDeletion(), true);

    delete m_buffer;
}

} // namespace Network
} // namespace Kitsunemimi
