#pragma once

#include <WinSock2.h>

// WSA.h - Provides wrapper functions for WSA library.

errno_t wsa_init(WSADATA** resultWsa);
errno_t wsa_cleanup(WSADATA* wsa);

errno_t wsa_openSocket(SOCKET* resultSocket);
errno_t wsa_closeSocket(SOCKET socket);

errno_t wsa_getCurrentAddressInfo(ADDRINFOA** resultAddrinfo, const char* serviceName);
errno_t wsa_initAddressInfo(ADDRINFOA* resultAddrinfo, SOCKADDR_IN address);
errno_t wsa_freeAddressInfo(ADDRINFOA* addrinfo);

errno_t wsa_bind(SOCKET socket, SOCKADDR_IN address);
errno_t wsa_listen(SOCKET socket);
errno_t wsa_accept(SOCKET* resultClientSocket, SOCKADDR_IN* resultClientAddress, SOCKET serverSocket);

errno_t wsa_connect(SOCKET socket, SOCKADDR_IN address);

errno_t wsa_send(SOCKET destinationSocket, const char* data, int count);
errno_t wsa_receive(SOCKET sourceSocket, char* data, int count);

SOCKADDR_IN makeAddress(BYTE a1, BYTE a2, BYTE a3, BYTE a4, unsigned short port);