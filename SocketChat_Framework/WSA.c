#include "WSA.h"
#include <stdio.h>

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

errno_t wsa_shutdownSocket(SOCKET socket)
{
	return shutdown(socket, SD_BOTH);
}

errno_t wsa_closeSocket(SOCKET socket)
{
	if (closesocket(socket) == SOCKET_ERROR)
	{
		return GetLastError();
	}
	return NOERROR;
}

errno_t wsa_bind(SOCKET socket, SOCKADDR_IN address)
{
	return (bind(socket, (SOCKADDR*)&address, sizeof(SOCKADDR_IN)) == SOCKET_ERROR) ? GetLastError() : NOERROR;
}

errno_t wsa_connect(SOCKET socket, SOCKADDR_IN address)
{
	return (connect(socket, (SOCKADDR*)&address, sizeof(SOCKADDR_IN)) == SOCKET_ERROR) ? GetLastError() : NOERROR;
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

errno_t wsa_send(SOCKET destinationSocket, const BYTE* data, size_t count)
{
	int bytes = send(destinationSocket, data, count, 0);
	//printf("<Sent %d bytes>\n", bytes);
	return (bytes == SOCKET_ERROR) ? GetLastError() : NOERROR;
}

errno_t wsa_receive(SOCKET sourceSocket, BYTE* data, size_t count)
{
	int bytes = recv(sourceSocket, data, count, 0);
	//printf("<Received %d bytes>\n", bytes);
	return (bytes == SOCKET_ERROR) ? GetLastError() : NOERROR;
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
	memset(&sockaddr.sin_zero, 0, 8);
	return sockaddr;
}