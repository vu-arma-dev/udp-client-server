// UDP Client Server -- send/receive UDP packets
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

// EDIT TO WORK WITH WINDOWS -- Garrison Johnston 11/1/2024

#ifndef SNAP_UDP_CLIENT_SERVER_H
#define SNAP_UDP_CLIENT_SERVER_H

#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <stdexcept>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

namespace udp_client_server
{

class UdpClientServerRuntimeError : public std::runtime_error
{
public:
    UdpClientServerRuntimeError(const char *w) : std::runtime_error(w) {}
};


class UdpClient
{
public:
                        UdpClient(const std::string& addr, int port);
                        ~UdpClient();

    int                 getSocket() const;
    int                 getPort() const;
    std::string         getAddr() const;

    int                 send(const char *msg, size_t size);

private:
    SOCKET              f_socket_;
    int                 f_port_;
    std::string         f_addr_;
    struct addrinfo *   f_addrinfo_;
};


class UdpServer
{
public:
                        UdpServer(const std::string& addr, int port);
                        ~UdpServer();

    int                 getSocket() const;
    int                 getPort() const;
    std::string         getAddr() const;

    int                 recv(char *msg, size_t max_size);
    int                 timedRecv(char *msg, size_t max_size, int max_wait_ms);

private:
    SOCKET              f_socket_;
    int                 f_port_;
    std::string         f_addr_;
    struct addrinfo *   f_addrinfo_;
};

} // namespace udp_client_server

#endif
// SNAP_UDP_CLIENT_SERVER_H
// vim: ts=4 sw=4 et