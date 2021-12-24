#pragma once

#include <WinSock2.h>
#include "query.h"

typedef struct sClient
{
	WSADATA* wsa;
	SOCKET socket;
	User user;
	void* chatThread;
} Client;

Client* client_init();
errno_t client_connect(Client* client, SOCKADDR_IN address);
BOOL client_enterConnectedServer(Client* client, const char* name);
BYTE client_query_disconnect(Client* client);
BYTE client_query_howManyUsers(Client* client, int* online, int* maxOnline);
BYTE client_query_listUsers(Client* client, User** users, int* userCount);
BYTE client_query_getUserByName(Client* client, const char* name, User* user);
BYTE client_query_getUserById(Client* client, id userId, User* user);
BYTE client_query_sendMessage(Client* client, id userId, const char* text);
BYTE client_query_getMessagesFromUser(Client* client, id userId, Message** messages, int* messageCount);
void client_dispose(Client* client);