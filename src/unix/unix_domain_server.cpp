/**
 *  @file    unix_domain_server.cpp
 *
 *  @author  Tobias Anker <tobias.anker@kitsunemimi.moe>
 *
 *  @copyright MIT License
 */

#include <libKitsunemimiNetwork/unix/unix_domain_socket.h>
#include <libKitsunemimiNetwork/unix/unix_domain_server.h>
#include <libKitsunemimiNetwork/net_socket.h>
#include <libKitsunemimiCommon/logger.h>

namespace Kitsunemimi
{
namespace Network
{

/**
 * @brief constructor
 *
 * @param socketFile file for the unix-domain-socket
 */
UnixDomainServer::UnixDomainServer(const std::string &socketFile)
{
    m_socketFile = socketFile;

    type = 1;
}

UnixDomainServer::UnixDomainServer()
{

}

/**
 * @brief destructor
 */
UnixDomainServer::~UnixDomainServer()
{
}

/**
 * @brief creates a server on a specific port
 *
 * @param error reference for error-output
 *
 * @return false, if server creation failed, else true
 */
bool
UnixDomainServer::initServer(ErrorContainer &error)
{
    // check file-path length to avoid conflics, when copy to the sockaddr_un-object
    if(m_socketFile.size() > 100)
    {
        error.addMeesage("Failed to create a unix-server, "
                         "because the filename is longer then 100 characters: \""
                         + m_socketFile
                         + "\"");
        error.addSolution("use a shorter name for the unix-domain-socket");
        return false;
    }

    // create socket
    serverFd = socket(AF_LOCAL, SOCK_STREAM, 0);
    if(serverFd < 0)
    {
        error.addMeesage("Failed to create a unix-socket");
        error.addSolution("Maybe no permissions to create a unix-socket on the system");
        return false;
    }

    unlink(m_socketFile.c_str());
    m_server.sun_family = AF_LOCAL;
    strncpy(m_server.sun_path, m_socketFile.c_str(), m_socketFile.size());
    m_server.sun_path[m_socketFile.size()] = '\0';

    // bind to port
    if(bind(serverFd, reinterpret_cast<struct sockaddr*>(&m_server), sizeof(m_server)) < 0)
    {
        error.addMeesage("Failed to bind unix-socket to addresse: \"" + m_socketFile + "\"");
        return false;
    }

    // start listening for incoming connections
    if(listen(serverFd, 5) == -1)
    {
        error.addMeesage("Failed listen on unix-socket on addresse: \"" + m_socketFile + "\"");
        return false;
    }

    LOG_INFO("Successfully initialized unix-socket server on targe: " + m_socketFile);

    return true;
}

/**
 * @brief wait for new incoming unix-socket-connections
 *
 * @param error reference for error-output
 */
bool
UnixDomainServer::waitForIncomingConnection(bool* abort,
                                            ErrorContainer &error)
{
    uint32_t length = sizeof(struct sockaddr_un);

    //make new connection
    const int fd = accept(serverFd, reinterpret_cast<struct sockaddr*>(&m_server), &length);

    if(*abort) {
        return true;
    }

    if(fd < 0)
    {
        error.addMeesage("Failed accept incoming connection on unix-server with address: \""
                         + m_socketFile
                         + "\"");
        return false;
    }

    LOG_INFO("Successfully accepted incoming connection on unix-socket server with address: \""
             + m_socketFile
             + "\"");

    // create new socket-object from file-descriptor
    const std::string name = "UDS_socket";    
    UnixDomainSocket unixSocket(fd);
    NetSocket<UnixDomainSocket>* netSocket = new NetSocket<UnixDomainSocket>(std::move(unixSocket),
                                                                             name);
    m_processConnection(m_target, netSocket);

    return true;
}

} // namespace Network
} // namespace Kitsunemimi

