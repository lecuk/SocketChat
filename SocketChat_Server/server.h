#pragma once
#include "WSA.h"
#include "llist.h"
#include "user.h"
#include "response.h"

#define SERVER_MAX_CLIENTS 4

typedef struct sServer
{
	WSADATA* wsa;
	SOCKET socket;
	SOCKADDR_IN address;
	LinkedList* clients;
	void* clientsMutex;
	void* listenThread;
	unsigned long listenThreadId;
} Server;

typedef struct sClientInfo
{
	Server* server;
	SOCKET socket;
	SOCKADDR_IN address;
	User user;
	LinkedList* messages;
	void* messagesMutex;
	void* queryThread;
	unsigned long queryThreadId;
} ClientInfo;

Server* server_init(SOCKADDR_IN address);
Response server_handleQuery_enter(Server* server, ClientInfo* client);
Response server_handleQuery_disconnect(Server* server, ClientInfo* client);
Response server_handleQuery_howManyUsers(Server* server, ClientInfo* client);
Response server_handleQuery_listUsers(Server* server, ClientInfo* client);
Response server_handleQuery_sendMessage(Server* server, ClientInfo* client);
Response server_handleQuery_getMessagesFromUser(Server* server, ClientInfo* client);
Response server_handleQuery_getUserByName(Server* server, ClientInfo* client);
Response server_handleQuery_getUserById(Server* server, ClientInfo* client);
Response server_handleQuery_unknown(Server* server, ClientInfo* client);
void server_listen(Server* server);
ClientInfo* server_addNewClient(Server* server, SOCKET socket, SOCKADDR_IN address);
errno_t server_removeInactiveClient(Server* server, ClientInfo* client);
errno_t server_disconnectClient(Server* server, ClientInfo* client);
void server_dispose(Server* server);