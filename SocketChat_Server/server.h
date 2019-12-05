#pragma once
#include "WSA.h"
#include "llist.h"
#include "user.h"

#define MAX_CLIENTS 4

typedef struct sServer
{
	WSADATA* wsa;
	SOCKET socket; 
	ADDRINFOA* addrinfo;
	SOCKADDR_IN* address;
	char addressString[64];
	LinkedList* clients;
	void* mutex;
} Server;

typedef struct sClientInfo
{
	SOCKET socket;
	ADDRINFOA* address;
} ClientInfo;

Server* server_init(const char* service);
void server_run(Server* server);
void server_listen(Server* server);
void server_dispose(Server* server);