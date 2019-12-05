#include "WSA.h"
#include <stdio.h>
#include <ws2tcpip.h>

errno_t wsa_init(WSADATA** resultWsa)
{
	*resultWsa = malloc(sizeof(WSADATA));
	return WSAStartup(0x0202, *resultWsa);
}

errno_t wsa_cleanup(WSADATA* wsa)
{
	if (!wsa) return ERROR_INVALID_HANDLE;
	free(wsa);

	return WSACleanup();
}

errno_t wsa_openSocket(SOCKET* resultSocket)
{
	return ((*resultSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET) ? GetLastError() : NOERROR;
}

errno_t wsa_closeSocket(SOCKET socket)
{
	if (closesocket(socket) == SOCKET_ERROR)
	{
		return GetLastError();
	}
	return NOERROR;
}

errno_t wsa_getCurrentAddressInfo(ADDRINFOA** resultAddrinfo, const char* serviceName)
{
	ADDRINFOA hints;

	memset(&hints, 0, sizeof(ADDRINFOA));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;
	return getaddrinfo(NULL, serviceName, &hints, resultAddrinfo);
}

errno_t wsa_initAddressInfo(ADDRINFOA* resultAddrinfo, SOCKADDR_IN address)
{
	resultAddrinfo->ai_family = AF_INET;
	resultAddrinfo->ai_socktype = SOCK_STREAM;
	resultAddrinfo->ai_protocol = IPPROTO_TCP;
	resultAddrinfo->ai_flags = 0;
	resultAddrinfo->ai_next = NULL;
	resultAddrinfo->ai_addrlen = sizeof(SOCKADDR_IN);
	resultAddrinfo->ai_canonname = NULL;
	memcpy(resultAddrinfo->ai_addr, &address, sizeof(SOCKADDR_IN));
	return NOERROR;
}

errno_t wsa_freeAddressInfo(ADDRINFOA* addrinfo)
{
	if (!addrinfo) return ERROR_INVALID_HANDLE;
	freeaddrinfo(addrinfo);
	return NOERROR;
}

errno_t wsa_bind(SOCKET socket, SOCKADDR_IN address)
{
	return (bind(socket, (SOCKADDR*)&address, sizeof(address)) == SOCKET_ERROR) ? GetLastError() : NOERROR;
}

errno_t wsa_connect(SOCKET socket, SOCKADDR_IN address)
{
	return (connect(socket, (SOCKADDR*)&address, sizeof(address)) == SOCKET_ERROR) ? GetLastError() : NOERROR;
}

errno_t wsa_listen(SOCKET socket)
{
	return (listen(socket, SOMAXCONN) == SOCKET_ERROR) ? GetLastError() : NOERROR;
}

errno_t wsa_accept(SOCKET* resultClientSocket, SOCKADDR_IN* resultClientAddress, SOCKET serverSocket)
{
	int addrLen = sizeof(SOCKADDR_IN);
	return ((*resultClientSocket = accept(serverSocket, (SOCKADDR*)resultClientAddress, &addrLen)) == INVALID_SOCKET) ? GetLastError() : NOERROR;
}

errno_t wsa_send(SOCKET destinationSocket, const char* data, int count)
{
	return (send(destinationSocket, data, count, 0) == SOCKET_ERROR) ? GetLastError() : NOERROR;
}

errno_t wsa_receive(SOCKET sourceSocket, char* data, int count)
{
	return (recv(sourceSocket, data, count, 0) == SOCKET_ERROR) ? GetLastError() : NOERROR;
}

SOCKADDR_IN makeAddress(BYTE a1, BYTE a2, BYTE a3, BYTE a4, unsigned short port)
{
	SOCKADDR_IN sockaddr;
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_addr.S_un.S_un_b.s_b1 = a1;
	sockaddr.sin_addr.S_un.S_un_b.s_b2 = a2;
	sockaddr.sin_addr.S_un.S_un_b.s_b3 = a3;
	sockaddr.sin_addr.S_un.S_un_b.s_b4 = a4;
	sockaddr.sin_port = port;
	return sockaddr;
}