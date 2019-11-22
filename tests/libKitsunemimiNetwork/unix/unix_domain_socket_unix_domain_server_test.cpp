/**
 *  @file    unix_domain_socket_unix_domain_server_test.cpp
 *
 *  @author  Tobias Anker <tobias.anker@kitsunemimi.moe>
 *
 *  @copyright MIT License
 */

#include "unix_domain_socket_unix_domain_server_test.h"
#include <libKitsunemimiCommon/data_buffer.h>

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
                                  MessageRingBuffer* recvBuffer,
                                  AbstractSocket*)
{
    Common::DataBuffer* targetBuffer = static_cast<Common::DataBuffer*>(target);
    const uint8_t* dataPointer = getDataPointer(*recvBuffer, recvBuffer->readWriteDiff);

    if(dataPointer == nullptr) {
        return 0;
    }

    addDataToBuffer(targetBuffer, dataPointer, recvBuffer->readWriteDiff);
    return recvBuffer->readWriteDiff;
}

/**
 * processConnectionUnixDomain-callback
 */
void processConnectionUnixDomain(void* target,
                                 AbstractSocket* socket)
{
    socket->setMessageCallback(target, &processMessageUnixDomain);
    socket->startThread();
}


UnixDomainSocket_UnixDomainServer_Test::UnixDomainSocket_UnixDomainServer_Test() :
    Kitsunemimi::Common::Test("UnixDomainSocket_UnixDomainServer_Test")
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
UnixDomainSocket_UnixDomainServer_Test::initTestCase()
{
    m_buffer = new Common::DataBuffer(1000);
    m_server = new UnixDomainServer(m_buffer, &processConnectionUnixDomain);
}

/**
 * checkConnectionInit
 */
void
UnixDomainSocket_UnixDomainServer_Test::checkConnectionInit()
{
    // init server
    TEST_EQUAL(m_server->getType(), AbstractServer::UNIX_SERVER);
    TEST_EQUAL(m_server->initServer("/tmp/sock.uds"), true);
    TEST_EQUAL(m_server->startThread(), true);

    // init client
    m_socketClientSide = new UnixDomainSocket("/tmp/sock.uds");
    TEST_EQUAL(m_socketClientSide->initClientSide(), true);
    TEST_EQUAL(m_socketClientSide->initClientSide(), true);
    TEST_EQUAL(m_socketClientSide->getType(), AbstractSocket::UNIX_SOCKET);

    usleep(10000);

    TEST_EQUAL(m_server->getNumberOfSockets(), 1);

    if(m_server->getNumberOfSockets() == 1)
    {
        m_socketServerSide = static_cast<UnixDomainSocket*>(m_server->getPendingSocket());
        TEST_EQUAL(m_socketServerSide->getType(), AbstractSocket::UNIX_SOCKET);
        TEST_EQUAL(m_server->getNumberOfSockets(), 0);
    }
}

/**
 * checkLittleDataTransfer
 */
void
UnixDomainSocket_UnixDomainServer_Test::checkLittleDataTransfer()
{
    usleep(10000);

    std::string sendMessage("poipoipoi");
    TEST_EQUAL(m_socketClientSide->sendMessage(sendMessage), true);
    usleep(10000);
    TEST_EQUAL(m_buffer->bufferPosition, 9);


    if(m_buffer->bufferPosition == 9)
    {
        Common::DataBuffer* buffer = m_buffer;
        uint64_t bufferSize = buffer->bufferPosition;
        char recvMessage[bufferSize];
        memcpy(recvMessage, buffer->data, bufferSize);
        TEST_EQUAL(bufferSize, 9);
        TEST_EQUAL(recvMessage[2], sendMessage.at(2));
        resetBuffer(m_buffer, 1000);
    }
}

/**
 * checkBigDataTransfer
 */
void
UnixDomainSocket_UnixDomainServer_Test::checkBigDataTransfer()
{
    std::string sendMessage = "poi";
    TEST_EQUAL(m_socketClientSide->sendMessage(sendMessage), true);
    for(uint32_t i = 0; i < 99999; i++)
    {
        m_socketClientSide->sendMessage(sendMessage);
    }
    usleep(10000);
    uint64_t totalIncom = m_buffer->bufferPosition;
    Common::DataBuffer* dataBuffer = m_buffer;
    TEST_EQUAL(totalIncom, 300000);
    TEST_EQUAL(dataBuffer->bufferPosition, 300000);
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
UnixDomainSocket_UnixDomainServer_Test::cleanupTestCase()
{
    TEST_EQUAL(m_socketServerSide->closeSocket(), true);
    m_socketServerSide->closeSocket();
    TEST_EQUAL(m_server->closeServer(), true);

    delete m_server;
    delete m_buffer;
}

} // namespace Network
} // namespace Kitsunemimi