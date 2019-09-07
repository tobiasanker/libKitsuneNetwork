/**
 *  @file    main.cpp
 *
 *  @author  Tobias Anker <tobias.anker@kitsunemimi.moe>
 *
 *  @copyright MIT License
 */

#include <libKitsuneNetwork/tcp/tcp_socket_tcp_server_test.h>
#include <libKitsuneNetwork/unix/unix_socket_unix_server_test.h>
#include <libKitsuneNetwork/tls_tcp/tls_tcp_socket_tls_tcp_server_test.h>

int main()
{
    Kitsune::Network::TcpSocket_TcpServer_Test();
    Kitsune::Network::UnixSocket_UnixServer_Test();
    Kitsune::Network::TlsTcpSocket_TcpServer_Test();
}
