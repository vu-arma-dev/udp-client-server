// UDP Client & Server -- classes to ease handling sockets
// Copyright (C) 2013  Made to Order Software Corp.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

// Modifications:
// Reformated variable/method names to match the vanderbot_control style guide. 
// Make all UDP sockets non-blocking
// Andrew Orekhov, ARMA Lab, 2021
// EDIT TO WORK WITH WINDOWS -- Garrison Johnston 11/1/2024

#ifndef SNAP_UDP_CLIENT_SERVER_CPP
#define SNAP_UDP_CLIENT_SERVER_CPP

#include <udp_client_server.h>
#include <string.h>

namespace udp_client_server
{

// ========================= CLIENT =========================

/** \brief Initialize a UDP client object.
 *
 * This function initializes the UDP client object using the address and the
 * port as specified.
 *
 * The port is expected to be a host side port number (i.e. 59200).
 *
 * The \p addr parameter is a textual address. It may be an IPv4 or IPv6
 * address and it can represent a host name or an address defined with
 * just numbers. If the address cannot be resolved then an error occurs
 * and constructor throws.
 *
 * \note
 * The socket is open in this process. If you fork() or exec() then the
 * socket will be closesocketd by the operating system.
 *
 * \warning
 * We only make use of the first address found by getaddrinfo(). All
 * the other addresses are ignored.
 *
 * \exception UdpClientServerRuntimeError
 * The server could not be initialized properly. Either the address cannot be
 * resolved, the port is incompatible or not available, or the socket could
 * not be created.
 *
 * \param[in] addr  The address to convert to a numeric IP.
 * \param[in] port  The port number.
 */
UdpClient::UdpClient(const std::string& addr, int port)
    : f_port_(port)
    , f_addr_(addr)
{
    
    // Initialize Winsock
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) 
    {
        WSACleanup();
        std::cerr << "WSAStartup failed with error: " << result << std::endl;
    }

    char decimal_port[16];
    snprintf(decimal_port, sizeof(decimal_port), "%d", f_port_);
    decimal_port[sizeof(decimal_port) / sizeof(decimal_port[0]) - 1] = '\0';
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET; // Only IPv4
    hints.ai_socktype = SOCK_DGRAM; // Specifies UDP
    hints.ai_protocol = IPPROTO_UDP; // Specifies UDP
    int r(getaddrinfo(addr.c_str(), decimal_port, &hints, &f_addrinfo_));
    if(r != 0 || f_addrinfo_ == NULL)
    {
        throw UdpClientServerRuntimeError(("invalid address or port: \"" + addr + ":" + decimal_port + "\"").c_str());
    }
    f_socket_ = socket(f_addrinfo_->ai_family, SOCK_DGRAM, IPPROTO_UDP);
    if(f_socket_ == INVALID_SOCKET)
    {
        freeaddrinfo(f_addrinfo_);
        WSACleanup();
        throw UdpClientServerRuntimeError(("could not create socket for: \"" + addr + ":" + decimal_port + "\"").c_str());
    }

    // Set the socket to non-blocking mode
    u_long mode = 1; // 1 to enable non-blocking mode
    if (ioctlsocket(f_socket_, FIONBIO, &mode) != 0) 
    {
        WSACleanup();
        throw UdpClientServerRuntimeError("Failed to set non-blocking mode");
    }
}

/** \brief Clean up the UDP client object.
 *
 * This function frees the address information structure and closesocket the socket
 * before returning.
 */
UdpClient::~UdpClient()
{
    freeaddrinfo(f_addrinfo_);
    closesocket(f_socket_);
    WSACleanup(); // Clean up Winsock resources
}

/** \brief Retrieve a copy of the socket identifier.
 *
 * This function return the socket identifier as returned by the socket()
 * function. This can be used to change some flags.
 *
 * \return The socket used by this UDP client.
 */
int UdpClient::getSocket() const
{
    return f_socket_;
}

/** \brief Retrieve the port used by this UDP client.
 *
 * This function returns the port used by this UDP client. The port is
 * defined as an integer, host side.
 *
 * \return The port as expected in a host integer.
 */
int UdpClient::getPort() const
{
    return f_port_;
}

/** \brief Retrieve a copy of the address.
 *
 * This function returns a copy of the address as it was specified in the
 * constructor. This does not return a canonalized version of the address.
 *
 * The address cannot be modified. If you need to send data on a different
 * address, create a new UDP client.
 *
 * \return A string with a copy of the constructor input address.
 */
std::string UdpClient::getAddr() const
{
    return f_addr_;
}

/** \brief Send a message through this UDP client.
 *
 * This function sends \p msg through the UDP client socket. The function
 * cannot be used to change the destination as it was defined when creating
 * the UdpClient object.
 *
 * The size must be small enough for the message to fit. In most cases we
 * use these in Snap! to send very small signals (i.e. 4 bytes commands.)
 * Any data we would want to share remains in the Cassandra database so
 * that way we can avoid losing it because of a UDP message.
 *
 * \param[in] msg  The message to send.
 * \param[in] size  The number of bytes representing this message.
 *
 * \return -1 if an error occurs, otherwise the number of bytes sent. errno
 * is set accordingly on error.
 */
int UdpClient::send(const char *msg, size_t size)
{
    int bytes_sent = sendto(f_socket_, msg, size, 0, f_addrinfo_->ai_addr, f_addrinfo_->ai_addrlen);
    if (bytes_sent == SOCKET_ERROR) 
    {
        int error = WSAGetLastError();
        std::cerr << "sendto failed with error: " << error << std::endl;
    }
    return bytes_sent;
}



// ========================= SERVER =========================

/** \brief Initialize a UDP server object.
 *
 * This function initializes a UDP server object making it ready to
 * receive messages.
 *
 * The server address and port are specified in the constructor so
 * if you need to receive messages from several different addresses
 * and/or port, you'll have to create a server for each.
 *
 * The address is a string and it can represent an IPv4 or IPv6
 * address.
 *
 * Note that this function calls connect() to connect the socket
 * to the specified address. To accept data on different UDP addresses
 * and ports, multiple UDP servers must be created.
 *
 * \note
 * The socket is open in this process. If you fork() or exec() then the
 * socket will be closesocketd by the operating system.
 *
 * \warning
 * We only make use of the first address found by getaddrinfo(). All
 * the other addresses are ignored.
 *
 * \exception UdpClient_server_runtime_error
 * The UdpClientServerRuntimeError exception is raised when the address
 * and port combinaison cannot be resolved or if the socket cannot be
 * opened.
 *
 * \param[in] addr  The address we receive on.
 * \param[in] port  The port we receive from.
 */
UdpServer::UdpServer(const std::string& addr, int port)
    : f_port_(port)
    , f_addr_(addr)
{
    // Initialize Winsock
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) 
    {
        WSACleanup();
        std::cerr << "WSAStartup failed with error: " << result << std::endl;
    }

    char decimal_port[16];
    snprintf(decimal_port, sizeof(decimal_port), "%d", f_port_);
    decimal_port[sizeof(decimal_port) / sizeof(decimal_port[0]) - 1] = '\0';
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET; // Only IPv4
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;
    int r(getaddrinfo(addr.c_str(), decimal_port, &hints, &f_addrinfo_));
    if(r != 0 || f_addrinfo_ == NULL)
    {
        WSACleanup();
        throw UdpClientServerRuntimeError(("invalid address or port for UDP socket: \"" + addr + ":" + decimal_port + "\"").c_str());
    }
    f_socket_ = socket(f_addrinfo_->ai_family, SOCK_DGRAM, IPPROTO_UDP);
    if(f_socket_ == INVALID_SOCKET)
    {
        freeaddrinfo(f_addrinfo_);
        WSACleanup();
        throw UdpClientServerRuntimeError(("could not create UDP socket for: \"" + addr + ":" + decimal_port + "\"").c_str());
    }
    // Set the socket to non-blocking mode
    u_long mode = 1; // 1 to enable non-blocking mode
    if (ioctlsocket(f_socket_, FIONBIO, &mode) != 0) 
    {
        WSACleanup();
        throw UdpClientServerRuntimeError("Failed to set non-blocking mode");
    }

    r = bind(f_socket_, f_addrinfo_->ai_addr, f_addrinfo_->ai_addrlen);
    if(r != 0)
    {
        freeaddrinfo(f_addrinfo_);
        closesocket(f_socket_);
        WSACleanup(); // Clean up Winsock resources
        throw UdpClientServerRuntimeError(("could not bind UDP socket with: \"" + addr + ":" + decimal_port + ". errno: " + std::to_string(errno) + "\"").c_str());
    }
}

/** \brief Clean up the UDP server.
 *
 * This function frees the address info structures and closes the socket.
 */
UdpServer::~UdpServer()
{
    freeaddrinfo(f_addrinfo_);
    closesocket(f_socket_);
    WSACleanup(); // Clean up Winsock resources
}

/** \brief The socket used by this UDP server.
 *
 * This function returns the socket identifier. It can be useful if you are
 * doing a select() on many sockets.
 *
 * \return The socket of this UDP server.
 */
int UdpServer::getSocket() const
{
    return f_socket_;
}

/** \brief The port used by this UDP server.
 *
 * This function returns the port attached to the UDP server. It is a copy
 * of the port specified in the constructor.
 *
 * \return The port of the UDP server.
 */
int UdpServer::getPort() const
{
    return f_port_;
}

/** \brief Return the address of this UDP server.
 *
 * This function returns a verbatim copy of the address as passed to the
 * constructor of the UDP server (i.e. it does not return the canonalized
 * version of the address.)
 *
 * \return The address as passed to the constructor.
 */
std::string UdpServer::getAddr() const
{
    return f_addr_;
}

/** \brief Attempt to receive a message in a non-blocking manner.
 *  
 * If no messages are available, -1 is returned and errno is set.
 *
 * \param[in] msg  The buffer where the message is saved.
 * \param[in] max_size  The maximum size the message (i.e. size of the \p msg buffer.)
 *
 * \return The number of bytes read or -1 if an error occurs.
 */
int UdpServer::recv(char *msg, size_t max_size)
{
    return ::recv(f_socket_, msg, max_size, 0);
}

} // namespace udp_client_server
#endif
// vim: ts=4 sw=4 et

