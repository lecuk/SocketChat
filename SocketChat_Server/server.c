#include "server.h"
#include <stdio.h>
#include "error_printer.h"
#include <WS2tcpip.h>
#include <WinSock2.h>
#include "update.h"
#include "query.h"
#include "response.h"

static id lastUserId = 0;

Server* server_init(SOCKADDR_IN address)
{
	Server* server = malloc(sizeof(Server));
	errno_t err;

	if ((err = wsa_init(&server->wsa)) == NOERROR)
	{
		printf("[%s ver.%d] has been loaded.\n", server->wsa->szDescription, server->wsa->wVersion);
	}
	else
	{
		errno_saveErrorMessage(err);
		printf("An error occured when loading WSA: (%d) %ws\n", err, errno_errMessage);
		free(server);
		return NULL;
	}

	server->address = address;
	printf("Server address: %hhu.%hhu.%hhu.%hhu:%hu\n",
		server->address.sin_addr.S_un.S_un_b.s_b1,
		server->address.sin_addr.S_un.S_un_b.s_b2,
		server->address.sin_addr.S_un.S_un_b.s_b3,
		server->address.sin_addr.S_un.S_un_b.s_b4,
		server->address.sin_port);

	if ((err = wsa_openSocket(&server->socket)) == NOERROR)
	{
		printf("Socket with handle 0x%04x was opened.\n", server->socket);
	}
	else
	{
		errno_saveErrorMessage(err);
		printf("An error occured when loading address: (%d) %ws\n", err, errno_errMessage);
		wsa_cleanup(server->wsa);
		free(server);
		return NULL;
	}

	if ((err = wsa_bind(server->socket, server->address)) == NOERROR)
	{
		printf("Socket 0x%04x was bound to address %hhu.%hhu.%hhu.%hhu:%hu\n", server->socket,
			server->address.sin_addr.S_un.S_un_b.s_b1,
			server->address.sin_addr.S_un.S_un_b.s_b2,
			server->address.sin_addr.S_un.S_un_b.s_b3,
			server->address.sin_addr.S_un.S_un_b.s_b4,
			server->address.sin_port);
	}
	else
	{
		errno_saveErrorMessage(err);
		printf("An error occured when binding socket: (%d) %ws\n", err, errno_errMessage);
		wsa_closeSocket(server->socket);
		wsa_cleanup(server->wsa);
		free(server);
		return NULL;
	}

	server->clients = llist_create(NULL, FALSE, sizeof(ClientInfo*));
	server->clientsMutex = CreateMutexA(NULL, FALSE, NULL);
	if (server->clientsMutex)
	{
		printf("Client mutex 0x%04x was created\n", (unsigned long)server->clientsMutex);
	}
	else
	{
		errno_saveErrorMessage(GetLastError());
		printf("An error occured when creating client mutex: (%d) %ws\n", err, errno_errMessage);
		llist_dispose(server->clients);
		wsa_closeSocket(server->socket);
		wsa_cleanup(server->wsa);
		free(server);
		return NULL;
	}

	return server;
}

Response server_handleQuery_enter(Server* server, ClientInfo* client)
{
	Response response;
	errno_t err;
	int curClientCount;

	curClientCount = server->clients->count;

	if ((err = wsa_receive(client->socket, (BYTE*)client->user.name, USER_NAME_LEN)) != NOERROR)
	{
		errno_saveErrorMessage(err);
		printf("An error occured when receiving data: (%d) %ws\n", err, errno_errMessage);
		response.type = RESPONSE_ERROR;
		memset(&response.singleResponse.data, 0, RESPONSE_DATA_LEN);
		response.multiResponse.count = 0;
		response.multiResponse.dataSize = 0;
		strcpy_s(response.message, RESPONSE_MESSAGE_LEN, "Can't get name");
		return response;
	}

	if (curClientCount >= SERVER_MAX_CLIENTS)
	{
		response.type = RESPONSE_NO;
		memset(&response.singleResponse.data, 0, RESPONSE_DATA_LEN);
		response.multiResponse.count = 0;
		response.multiResponse.dataSize = 0;
		strcpy_s(response.message, RESPONSE_MESSAGE_LEN, "Server is full");
		return response;
	}

	WaitForSingleObject(server->clientsMutex, INFINITE);
	llist_foreach(server->clients, client)
	{
		ClientInfo* iClient = client_node->item;

		if (!strcmp(iClient->user.name, client->user.name))
		{
			response.type = RESPONSE_NO;
			memset(&response.singleResponse.data, 0, RESPONSE_DATA_LEN);
			response.multiResponse.count = 0;
			response.multiResponse.dataSize = 0;
			strcpy_s(response.message, RESPONSE_MESSAGE_LEN, "Name already exists");
			return response;
		}
	}
	llist_add(server->clients, client);
	ReleaseMutex(server->clientsMutex);

	client->user.state = USERSTATE_ONLINE;

	printf("Client #%hu \"%s\" connected.\n", client->user.id, client->user.name);
	response.type = RESPONSE_YES;
	response.singleResponse.id = client->user.id;
	response.multiResponse.count = 0;
	response.multiResponse.dataSize = 0;
	strcpy_s(response.message, RESPONSE_MESSAGE_LEN, "Welcome to the server!");
	return response;
}

Response server_handleQuery_disconnect(Server* server, ClientInfo* client)
{
	Response response;
	int curClientCount;

	curClientCount = server->clients->count;

	if (curClientCount <= 0)
	{
		printf("How did count of clients became less than 0?\n");
		response.type = RESPONSE_ERROR;
		memset(&response.singleResponse.data, 0, RESPONSE_DATA_LEN);
		strcpy_s(response.message, RESPONSE_MESSAGE_LEN, "Server is empty");
		return response;
	}

	printf("Client #%hu \"%s\" is disconnecting.\n", client->user.id, client->user.name);
	response.type = RESPONSE_YES;
	memset(&response.singleResponse.data, 0, RESPONSE_DATA_LEN);
	response.multiResponse.count = 0;
	response.multiResponse.dataSize = 0;
	strcpy_s(response.message, RESPONSE_MESSAGE_LEN, "Goodbye");
	return response;
}

Response server_handleQuery_howManyUsers(Server* server, ClientInfo* client)
{
	Response response;

	response.type = RESPONSE_OK;
	response.singleResponse.userCount.online = server->clients->count;
	response.singleResponse.userCount.maxOnline = SERVER_MAX_CLIENTS;
	response.multiResponse.count = 0;
	response.multiResponse.dataSize = 0;
	strcpy_s(response.message, RESPONSE_MESSAGE_LEN, "ok");
	return response;
}

Response server_handleQuery_listUsers(Server* server, ClientInfo* client)
{
	Response response;

	response.type = RESPONSE_OK;
	memset(&response.singleResponse.data, 0, RESPONSE_DATA_LEN);
	response.multiResponse.count = server->clients->count;
	response.multiResponse.dataSize = sizeof(User);
	strcpy_s(response.message, RESPONSE_MESSAGE_LEN, "ok");
	return response;
}

Response server_handleQuery_sendMessage(Server* server, ClientInfo* client)
{
	errno_t err;
	Response response;
	Message message;
	ClientInfo* clientToSend = NULL;

	if ((err = wsa_receive(client->socket, (BYTE*)&message, sizeof(Message))) != NOERROR)
	{
		errno_saveErrorMessage(err);
		printf("An error occured when receiving data: (%d) %ws\n", err, errno_errMessage);
		response.type = RESPONSE_ERROR;
		memset(&response.singleResponse.data, 0, RESPONSE_DATA_LEN);
		response.multiResponse.count = 0;
		response.multiResponse.dataSize = 0;
		strcpy_s(response.message, RESPONSE_MESSAGE_LEN, "Can't get message");
		return response;
	}

	if (message.sender == message.receiver)
	{
		response.type = RESPONSE_NO;
		memset(&response.singleResponse.data, 0, RESPONSE_DATA_LEN);
		response.multiResponse.count = 0;
		response.multiResponse.dataSize = 0;
		strcpy_s(response.message, RESPONSE_MESSAGE_LEN, "Sender = receiver");
		return response;
	}

	WaitForSingleObject(server->clientsMutex, INFINITE);
	llist_foreach(server->clients, client)
	{
		ClientInfo* iClient = client_node->item;
		if (iClient->user.id == message.receiver)
		{
			clientToSend = iClient;
			break;
		}
	}
	ReleaseMutex(server->clientsMutex);

	if (!clientToSend)
	{
		response.type = RESPONSE_NO;
		memset(&response.singleResponse.data, 0, RESPONSE_DATA_LEN);
		response.multiResponse.count = 0;
		response.multiResponse.dataSize = 0;
		strcpy_s(response.message, RESPONSE_MESSAGE_LEN, "No such id to send to");
		return response;
	}

	WaitForSingleObject(clientToSend->messagesMutex, INFINITE);
	llist_add(clientToSend->messages, &message);
	ReleaseMutex(clientToSend->messagesMutex);

	response.type = RESPONSE_YES;
	memset(&response.singleResponse.data, 0, RESPONSE_DATA_LEN);
	response.multiResponse.count = 0;
	response.multiResponse.dataSize = 0;
	strcpy_s(response.message, RESPONSE_MESSAGE_LEN, "sent");
	return response;
}

Response server_handleQuery_getMessagesFromUser(Server* server, ClientInfo* client)
{
	errno_t err;
	Response response;
	id userId;

	if ((err = wsa_receive(client->socket, (BYTE*)&userId, sizeof(id))) != NOERROR)
	{
		errno_saveErrorMessage(err);
		printf("An error occured when receiving data: (%d) %ws\n", err, errno_errMessage);
		response.type = RESPONSE_ERROR;
		memset(&response.singleResponse.data, 0, RESPONSE_DATA_LEN);
		response.multiResponse.count = 0;
		response.multiResponse.dataSize = 0;
		strcpy_s(response.message, RESPONSE_MESSAGE_LEN, "Can't get id");
		return response;
	}

	if (userId == client->user.id)
	{
		response.type = RESPONSE_NO;
		memset(&response.singleResponse.data, 0, RESPONSE_DATA_LEN);
		response.multiResponse.count = 0;
		response.multiResponse.dataSize = 0;
		strcpy_s(response.message, RESPONSE_MESSAGE_LEN, "You are trying to get message from yourself");
		return response;
	}

	size_t messageCount = 0;
	WaitForSingleObject(client->messagesMutex, INFINITE);
	llist_foreach(client->messages, message)
	{
		Message* message = message_node->item;
		if (message->receiver == client->user.id) ++messageCount;
	}
	ReleaseMutex(client->messagesMutex);

	response.type = RESPONSE_YES;
	memset(&response.singleResponse.data, 0, RESPONSE_DATA_LEN);
	response.multiResponse.count = messageCount;
	response.multiResponse.dataSize = sizeof(Message);
	strcpy_s(response.message, RESPONSE_MESSAGE_LEN, "ok");
	return response;
}

Response server_handleQuery_getUserByName(Server* server, ClientInfo* client)
{
	errno_t err;
	Response response;
	char name[USER_NAME_LEN];

	if ((err = wsa_receive(client->socket, (BYTE*)&name, USER_NAME_LEN)) != NOERROR)
	{
		errno_saveErrorMessage(err);
		printf("An error occured when receiving data: (%d) %ws\n", err, errno_errMessage);
		response.type = RESPONSE_ERROR;
		memset(&response.singleResponse.data, 0, RESPONSE_DATA_LEN);
		response.multiResponse.count = 0;
		response.multiResponse.dataSize = 0;
		strcpy_s(response.message, RESPONSE_MESSAGE_LEN, "Can't get name");
		return response;
	}

	int noSuchUser = TRUE;
	WaitForSingleObject(server->clientsMutex, INFINITE);
	llist_foreach(server->clients, client)
	{
		ClientInfo* iClient = client_node->item;
		if (!strcmp(iClient->user.name, name))
		{
			noSuchUser = FALSE;
			response.singleResponse.user = iClient->user;
			break;
		}
	}
	ReleaseMutex(server->clientsMutex);

	if (noSuchUser)
	{
		response.type = RESPONSE_NO;
		memset(&response.singleResponse.data, 0, RESPONSE_DATA_LEN);
		response.multiResponse.count = 0;
		response.multiResponse.dataSize = 0;
		strcpy_s(response.message, RESPONSE_MESSAGE_LEN, "No such user");
		return response;
	}

	response.type = RESPONSE_YES;
	response.multiResponse.count = 0;
	response.multiResponse.dataSize = 0;
	strcpy_s(response.message, RESPONSE_MESSAGE_LEN, "ok");
	return response;
}

Response server_handleQuery_getUserById(Server* server, ClientInfo* client)
{
	errno_t err;
	Response response;
	id id;

	if ((err = wsa_receive(client->socket, (BYTE*)&id, sizeof(id))) != NOERROR)
	{
		errno_saveErrorMessage(err);
		printf("An error occured when receiving data: (%d) %ws\n", err, errno_errMessage);
		response.type = RESPONSE_ERROR;
		memset(&response.singleResponse.data, 0, RESPONSE_DATA_LEN);
		response.multiResponse.count = 0;
		response.multiResponse.dataSize = 0;
		strcpy_s(response.message, RESPONSE_MESSAGE_LEN, "Can't get name");
		return response;
	}

	int noSuchUser = TRUE;
	WaitForSingleObject(server->clientsMutex, INFINITE);
	llist_foreach(server->clients, client)
	{
		ClientInfo* iClient = client_node->item;
		if (iClient->user.id == id)
		{
			noSuchUser = FALSE;
			response.singleResponse.user = iClient->user;
			break;
		}
	}
	ReleaseMutex(server->clientsMutex);

	if (noSuchUser)
	{
		response.type = RESPONSE_NO;
		memset(&response.singleResponse.data, 0, RESPONSE_DATA_LEN);
		response.multiResponse.count = 0;
		response.multiResponse.dataSize = 0;
		strcpy_s(response.message, RESPONSE_MESSAGE_LEN, "No such user");
		return response;
	}

	response.type = RESPONSE_YES;
	response.multiResponse.count = 0;
	response.multiResponse.dataSize = 0;
	strcpy_s(response.message, RESPONSE_MESSAGE_LEN, "ok");
	return response;
}

Response server_handleQuery_unknown(Server* server, ClientInfo* client)
{
	Response response;

	response.type = RESPONSE_ERROR;
	memset(&response.singleResponse.data, 0, RESPONSE_DATA_LEN);
	response.multiResponse.count = 0;
	response.multiResponse.dataSize = 0;
	strcpy_s(response.message, RESPONSE_MESSAGE_LEN, "Unknown query");
	return response;
}

unsigned long __stdcall server_handleClientAsync(void* clientRaw)
{
	ClientInfo* client = clientRaw;
	Server* server = client->server;
	errno_t err;
	Response response;

	while (1)
	{
		Query query;
		if ((err = wsa_receive(client->socket, (BYTE*)&query, sizeof(Query))) == NOERROR)
		{
			switch (query.type)
			{
			case QUERY_CONNECT:
				response = server_handleQuery_enter(server, client);
				break;

			case QUERY_DISCONNECT:
				response = server_handleQuery_disconnect(server, client);
				break;

			case QUERY_HOWMANYUSERS:
				response = server_handleQuery_howManyUsers(server, client);
				break;

			case QUERY_LISTUSERS:
				response = server_handleQuery_listUsers(server, client);
				break;

			case QUERY_SENDMESSAGE:
				response = server_handleQuery_sendMessage(server, client);
				break;

			case QUERY_GETMESSAGESFROMUSER:
				response = server_handleQuery_getMessagesFromUser(server, client);
				break;

			case QUERY_GETUSERBYNAME:
				response = server_handleQuery_getUserByName(server, client);
				break;

			case QUERY_GETUSERBYID:
				response = server_handleQuery_getUserById(server, client);
				break;

			default:
				response = server_handleQuery_unknown(server, client);
				break;
			}

			printf("Client #%hu sent query with type %hhu and argument size %u. Response is of type %hhu and message: \"%s\"\n", 
				client->user.id, query.type, query.dataSize, response.type, response.message);
			if ((err = wsa_send(client->socket, (BYTE*)&response, sizeof(Response))) != NOERROR)
			{
				errno_saveErrorMessage(err);
				printf("An error occured when sending response data: (%d) %ws\n", err, errno_errMessage);
				server_removeInactiveClient(server, client);
				return 0;
			}

			switch (query.type)
			{
			case QUERY_CONNECT:
				break;

			case QUERY_DISCONNECT:
				server_disconnectClient(server, client);
				return 0;

			case QUERY_HOWMANYUSERS:
				break;

			case QUERY_LISTUSERS:
				WaitForSingleObject(server->clientsMutex, INFINITE);
				llist_foreach(server->clients, client)
				{
					ClientInfo* iClient = client_node->item;
					if ((err = wsa_send(client->socket, (BYTE*)&iClient->user, sizeof(User))) != NOERROR)
					{
						errno_saveErrorMessage(err);
						printf("An error occured when sending response data: (%d) %ws\n", err, errno_errMessage);
						server_removeInactiveClient(server, client);
						return 0;
					}
				}
				ReleaseMutex(server->clientsMutex);
				break;

			case QUERY_SENDMESSAGE:
				++client->user.messagesSent;
				break;

			case QUERY_GETMESSAGESFROMUSER:
				WaitForSingleObject(client->messagesMutex, INFINITE);
				LinkedListNode* messageNode = client->messages->firstNode;
				LinkedListNode* nextNode = NULL;
				int messageCount = response.multiResponse.count;
				printf("Messages for %s: %d\n", client->user.name, messageCount);
				while (messageNode != NULL && messageCount > 0)
				{
					nextNode = messageNode->nextNode;
					Message* message = messageNode->item;
					printf("Message [%d]: %s\n", messageCount, message->text);
					if (message->receiver == client->user.id)
					{
						if ((err = wsa_send(client->socket, (BYTE*)message, sizeof(Message))) != NOERROR)
						{
							errno_saveErrorMessage(err);
							printf("An error occured when sending response data: (%d) %ws\n", err, errno_errMessage);
							server_removeInactiveClient(server, client);
							return 0;
						}
						--messageCount;
						llist_remove(client->messages, message); //error is probably here?
					}
					messageNode = nextNode;
				}
				ReleaseMutex(client->messagesMutex);

				client->user.messagesReceived += response.multiResponse.count;
				break;

			case QUERY_GETUSERBYNAME:
				break;

			case QUERY_GETUSERBYID:
				break;

			default:
				break;
			}
		}
		else
		{
			errno_saveErrorMessage(err);
			printf("An error occured when receiving data: (%d) %ws\n", err, errno_errMessage);
			server_removeInactiveClient(server, client);
			return 0;
		}
	}
}

unsigned long __stdcall server_listenAsync(void* serverRaw)
{
	Server* server = serverRaw;
	errno_t err;
	while (1)
	{
		SOCKET clientSocket;
		SOCKADDR_IN clientAddress;

		if ((err = wsa_accept(&clientSocket, &clientAddress, server->socket)) == NOERROR)
		{
			printf("Accepted a new client! Socket = 0x%04x, address: %hhu.%hhu.%hhu.%hhu:%hu\n",
				clientSocket,
				clientAddress.sin_addr.S_un.S_un_b.s_b1,
				clientAddress.sin_addr.S_un.S_un_b.s_b2,
				clientAddress.sin_addr.S_un.S_un_b.s_b3,
				clientAddress.sin_addr.S_un.S_un_b.s_b4,
				clientAddress.sin_port);
		}
		else
		{
			errno_saveErrorMessage(err);
			printf("An error occured when accepting client: (%d) %ws\n", err, errno_errMessage);
			return 0;
		}

		ClientInfo* clientInfo = server_addNewClient(server, clientSocket, clientAddress);
		if (clientInfo)
		{
			printf("Client info #%hu was created.\n", clientInfo->user.id);
		}
		else
		{
			errno_saveErrorMessage(err = GetLastError());
			printf("An error occured when creating client info: (%d) %ws\n", err, errno_errMessage);
			return 0;
		}
	}
}

void server_listen(Server* server)
{
	errno_t err;

	if ((err = wsa_listen(server->socket)) == NOERROR)
	{
		printf("Listening to socket 0x%04x...\n", server->socket);
	}
	else
	{
		errno_saveErrorMessage(err);
		printf("An error occured when listening to socket: (%d) %ws\n", err, errno_errMessage);
		return;
	}

	server->listenThread = CreateThread(NULL, 0, server_listenAsync, server, 0, &server->listenThreadId);
}

ClientInfo* server_addNewClient(Server* server, SOCKET socket, SOCKADDR_IN address)
{
	ClientInfo* client = malloc(sizeof(ClientInfo));
	if (!client) return NULL;
	client->server = server;
	client->socket = socket;
	client->address = address;
	client->user.name[0] = 0;
	client->user.state = USERSTATE_UNKNOWN;
	client->user.id = lastUserId++;
	client->user.messagesSent = 0;
	client->user.messagesReceived = 0;
	client->queryThread = CreateThread(NULL, 0, server_handleClientAsync, client, 0, &client->queryThreadId);
	client->messages = llist_create(NULL, TRUE, sizeof(Message));
	client->messagesMutex = CreateMutexA(NULL, FALSE, NULL);

	return client;
}

errno_t server_removeInactiveClient(Server* server, ClientInfo* client)
{
	if (!CloseHandle(client->queryThread)) return GetLastError();

	WaitForSingleObject(server->clientsMutex, INFINITE);
	llist_remove(server->clients, client);
	ReleaseMutex(server->clientsMutex);

	return NOERROR;
}

errno_t server_disconnectClient(Server* server, ClientInfo* client)
{
	errno_t err;

	if (client->user.state != USERSTATE_ONLINE) return NOERROR;
	client->user.state = USERSTATE_OFFLINE;

	if ((err = wsa_shutdownSocket(client->socket)) == NOERROR)
	{
		printf("Socket (client) 0x%04x was shut down.\n", client->socket);
	}
	else
	{
		errno_saveErrorMessage(err);
		printf("An error occured when shutting down socket: (%d) %ws\n", err, errno_errMessage);
		return err;
	}

	if ((err = wsa_closeSocket(client->socket)) == NOERROR)
	{
		printf("Socket (client) 0x%04x was closed.\n", client->socket);
	}
	else
	{
		errno_saveErrorMessage(err);
		printf("An error occured when closing socket: (%d) %ws\n", err, errno_errMessage);
		return err;
	}

	return NOERROR;
}

void server_dispose(Server* server)
{
	errno_t err;

	unsigned long exitCode;
	if (GetExitCodeThread(server->listenThread, &exitCode) && exitCode == STILL_ACTIVE)
	{
		if (TerminateThread(server->listenThread, 0))
		{
			printf("Listen thread was terminated.\n");
		}
		else
		{
			errno_saveErrorMessage(err = GetLastError());
			printf("An error occured when terminating listen thread: (%d) %ws\n", err, errno_errMessage);
		}
	}

	CloseHandle(server->listenThread);

	WaitForSingleObject(server->clientsMutex, INFINITE);
	LinkedListNode* clientNode = server->clients->firstNode;
	LinkedListNode* nextClientNode = NULL;
	while (clientNode)
	{
		nextClientNode = clientNode->nextNode;
		ClientInfo* client = clientNode->item;
		if (GetExitCodeThread(client->queryThread, &exitCode) && exitCode == STILL_ACTIVE)
		{
			if (TerminateThread(client->queryThread, 0))
			{
				printf("Query thread of client %hu was terminated.\n", client->user.id);
			}
			else
			{
				errno_saveErrorMessage(err = GetLastError());
				printf("An error occured when terminating query thread: (%d) %ws\n", err, errno_errMessage);
			}
		}
		server_disconnectClient(server, client);
		server_removeInactiveClient(server, client);
		clientNode = nextClientNode;
	}
	llist_clear(server->clients);
	ReleaseMutex(server->clientsMutex);

	if (CloseHandle(server->clientsMutex))
	{
		printf("Client mutex was closed.\n");
	}
	else
	{
		errno_saveErrorMessage(err);
		printf("An error occured when closing client mutex: (%d) %ws\n", err, errno_errMessage);
	}

	if ((err = wsa_closeSocket(server->socket)) == NOERROR)
	{
		printf("Socket 0x%04x was closed.\n", server->socket);
	}
	else
	{
		errno_saveErrorMessage(err);
		printf("An error occured when closing socket: (%d) %ws\n", err, errno_errMessage);
	}

	llist_dispose(server->clients);

	if ((err = wsa_cleanup(server->wsa)) == NOERROR)
	{
		printf("WSA cleaned up.\n");
	}
	else
	{
		errno_saveErrorMessage(err);
		printf("An error occured when cleaning WSA: (%d) %ws\n", err, errno_errMessage);
	}

	free(server);
}
