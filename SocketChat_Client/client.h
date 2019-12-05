#pragma once

#include <WinSock2.h>

typedef struct sClient
{
	WSADATA* wsa;
	SOCKET socket;
} Client;

Client* client_init(const char* name);
void client_dispose(Client* client);