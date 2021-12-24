#include "client.h"
#include "WSA.h"
#include <WinSock2.h>
#include "error_printer.h"
#include <stdio.h>
#include "response.h"

Client* client_init()
{
	errno_t err;
	Client* client = malloc(sizeof(Client));

	if ((err = wsa_init(&client->wsa)) == NOERROR)
	{
		printf("[%s ver.%d] has been loaded.\n", client->wsa->szDescription, client->wsa->wVersion);
	}
	else
	{
		errno_saveErrorMessage(err);
		printf("An error occured when loading WSA: (%d) %ws", err, errno_errMessage);
	}

	if ((err = wsa_openSocket(&client->socket)) == NOERROR)
	{
		printf("Socket with handle 0x%04x was opened.\n", client->socket);
	}
	else
	{
		errno_saveErrorMessage(err);
		printf("An error occured when loading address: (%d) %ws", err, errno_errMessage);
	}

	return client;
}

BOOL client_connect(Client* client, SOCKADDR_IN address)
{
	errno_t err;
	if ((err = wsa_connect(client->socket, address)) != NOERROR)
	{
		errno_saveErrorMessage(err);
		printf("An error occured when connecting to server: (%d) %ws", err, errno_errMessage);
		return FALSE;
	}

	return TRUE;
}

BOOL client_enterConnectedServer(Client* client, const char* name)
{
	errno_t err;
	Query query;
	Response response;

	strcpy_s(client->user.name, USER_NAME_LEN, name);

	query.type = QUERY_CONNECT;
	query.dataSize = USER_NAME_LEN;
	if ((err = wsa_send(client->socket, (BYTE*)&query, sizeof(Query))) != NOERROR)
	{
		errno_saveErrorMessage(err);
		printf("An error occured when sending data: (%d) %ws\n", err, errno_errMessage);
		return FALSE;
	}
	if ((err = wsa_send(client->socket, (BYTE*)client->user.name, query.dataSize)) != NOERROR)
	{
		errno_saveErrorMessage(err);
		printf("An error occured when sending data: (%d) %ws\n", err, errno_errMessage);
		return FALSE;
	}

	if ((err = wsa_receive(client->socket, (BYTE*)&response, sizeof(Response))) != NOERROR)
	{
		errno_saveErrorMessage(err);
		printf("An error occured when receiving data: (%d) %ws", err, errno_errMessage);
		return FALSE;
	}

	switch (response.type)
	{
	case RESPONSE_YES:
		client->user.id = response.singleResponse.id;
		return TRUE;

	case RESPONSE_NO:
		printf("Server denied: %s\n", response.message);
		return FALSE;

	case RESPONSE_ERROR:
		printf("Error: %s\n", response.message);
		return FALSE;

	case RESPONSE_NONE:
		printf("Server didn't respond\n");
		return FALSE;

	default:
		printf("Unexpected response: %s\n", response.message);
		return FALSE;
	}

	return FALSE;
}

BYTE client_query_disconnect(Client* client)
{
	errno_t err;
	Query query;
	Response response;
	query.type = QUERY_DISCONNECT;
	query.dataSize = 0;
	if ((err = wsa_send(client->socket, (BYTE*)&query, sizeof(Query))) != NOERROR)
	{
		errno_saveErrorMessage(err);
		printf("An error occured when sending data: (%d) %ws\n", err, errno_errMessage);
		return RESPONSE_NONE;
	}

	if ((err = wsa_receive(client->socket, (BYTE*)&response, sizeof(Response))) != NOERROR)
	{
		errno_saveErrorMessage(err);
		printf("An error occured when receiving data: (%d) %ws\n", err, errno_errMessage);
		return RESPONSE_NONE;
	}

	switch (response.type)
	{
	case RESPONSE_YES:
		printf("Disconnected from server.\n");
		break;

	case RESPONSE_NO:
		printf("Server denied: %s\n", response.message);
		break;

	case RESPONSE_ERROR:
		printf("Error: %s\n", response.message);
		break;

	case RESPONSE_NONE:
		printf("Server didn't respond\n");
		break;

	default:
		printf("Unexpected response: %s\n", response.message);
		break;
	}

	return response.type;
}

BYTE client_query_howManyUsers(Client* client, int* online, int* maxOnline)
{
	errno_t err;
	Query query;
	Response response;
	query.type = QUERY_HOWMANYUSERS;
	query.dataSize = 0;
	if ((err = wsa_send(client->socket, (BYTE*)&query, sizeof(Query))) != NOERROR)
	{
		errno_saveErrorMessage(err);
		printf("An error occured when sending data: (%d) %ws\n", err, errno_errMessage);
		return RESPONSE_NONE;
	}

	if ((err = wsa_receive(client->socket, (BYTE*)&response, sizeof(Response))) != NOERROR)
	{
		errno_saveErrorMessage(err);
		printf("An error occured when receiving data: (%d) %ws\n", err, errno_errMessage);
		return RESPONSE_NONE;
	}

	switch (response.type)
	{
	case RESPONSE_OK:
		*online = response.singleResponse.userCount.online;
		*maxOnline = response.singleResponse.userCount.maxOnline;
		break;

	case RESPONSE_ERROR:
		printf("Error: %s\n", response.message);
		break;

	case RESPONSE_NONE:
		printf("Server didn't respond\n");
		break;

	default:
		printf("Unexpected response: %s\n", response.message);
		break;
	}
	return response.type;
}

BYTE client_query_listUsers(Client* client, User** users, int* userCount)
{
	errno_t err;
	Query query;
	Response response;
	query.type = QUERY_LISTUSERS;
	query.dataSize = 0;
	if ((err = wsa_send(client->socket, (BYTE*)&query, sizeof(Query))) != NOERROR)
	{
		errno_saveErrorMessage(err);
		printf("An error occured when sending data: (%d) %ws\n", err, errno_errMessage);
		return RESPONSE_NONE;
	}

	if ((err = wsa_receive(client->socket, (BYTE*)&response, sizeof(Response))) != NOERROR)
	{
		errno_saveErrorMessage(err);
		printf("An error occured when receiving data: (%d) %ws\n", err, errno_errMessage);
		return RESPONSE_NONE;
	}

	switch (response.type)
	{
	case RESPONSE_OK:
		*userCount = response.multiResponse.count;
		*users = calloc(*userCount, sizeof(User));

		for (int i = 0; i < *userCount; ++i)
		{
			if ((err = wsa_receive(client->socket, (BYTE*)(*users + i), sizeof(User))) != NOERROR)
			{
				errno_saveErrorMessage(err);
				printf("An error occured when receiving data: (%d) %ws\n", err, errno_errMessage);
				return RESPONSE_ERROR;
			}
		}
		break;

	case RESPONSE_ERROR:
		printf("Error: %s\n", response.message);
		break;

	case RESPONSE_NONE:
		printf("Server didn't respond\n");
		break;

	default:
		printf("Unexpected response: %s\n", response.message);
		break;
	}
	return response.type;
}

BYTE client_query_getUserByName(Client* client, const char* name, User* user)
{
	errno_t err;
	Query query;
	Response response;
	query.type = QUERY_GETUSERBYNAME;
	query.dataSize = USER_NAME_LEN;

	if ((err = wsa_send(client->socket, (BYTE*)&query, sizeof(Query))) != NOERROR)
	{
		errno_saveErrorMessage(err);
		printf("An error occured when sending data: (%d) %ws\n", err, errno_errMessage);
		return RESPONSE_NONE;
	}

	if ((err = wsa_send(client->socket, (BYTE*)&name, USER_NAME_LEN)) != NOERROR)
	{
		errno_saveErrorMessage(err);
		printf("An error occured when sending additional data: (%d) %ws\n", err, errno_errMessage);
		return RESPONSE_NONE;
	}

	if ((err = wsa_receive(client->socket, (BYTE*)&response, sizeof(Response))) != NOERROR)
	{
		errno_saveErrorMessage(err);
		printf("An error occured when receiving data: (%d) %ws\n", err, errno_errMessage);
		return RESPONSE_NONE;
	}

	switch (response.type)
	{
	case RESPONSE_YES:
		*user = response.singleResponse.user;
		break;

	case RESPONSE_NO:
		printf("Server denied: %s\n", response.message);
		break;

	case RESPONSE_ERROR:
		printf("Error: %s\n", response.message);
		break;

	case RESPONSE_NONE:
		printf("Server didn't respond\n");
		break;

	default:
		printf("Unexpected response: %s\n", response.message);
		break;
	}
	return response.type;
}

BYTE client_query_getUserById(Client* client, id userId, User* user)
{
	errno_t err;
	Query query;
	Response response;
	query.type = QUERY_GETUSERBYID;
	query.dataSize = USER_NAME_LEN;

	if ((err = wsa_send(client->socket, (BYTE*)&query, sizeof(Query))) != NOERROR)
	{
		errno_saveErrorMessage(err);
		printf("An error occured when sending data: (%d) %ws\n", err, errno_errMessage);
		return RESPONSE_NONE;
	}

	if ((err = wsa_send(client->socket, (BYTE*)&userId, sizeof(id))) != NOERROR)
	{
		errno_saveErrorMessage(err);
		printf("An error occured when sending additional data: (%d) %ws\n", err, errno_errMessage);
		return RESPONSE_NONE;
	}

	if ((err = wsa_receive(client->socket, (BYTE*)&response, sizeof(Response))) != NOERROR)
	{
		errno_saveErrorMessage(err);
		printf("An error occured when receiving data: (%d) %ws\n", err, errno_errMessage);
		return RESPONSE_NONE;
	}

	switch (response.type)
	{
	case RESPONSE_YES:
		*user = response.singleResponse.user;
		break;

	case RESPONSE_NO:
		printf("Server denied: %s\n", response.message);
		break;

	case RESPONSE_ERROR:
		printf("Error: %s\n", response.message);
		break;

	case RESPONSE_NONE:
		printf("Server didn't respond\n");
		break;

	default:
		printf("Unexpected response: %s\n", response.message);
		break;
	}
	return response.type;
}

BYTE client_query_sendMessage(Client* client, id userId, const char* text)
{
	errno_t err;
	Query query;
	Response response;
	Message message;

	query.type = QUERY_SENDMESSAGE;
	query.dataSize = sizeof(Message);
	if ((err = wsa_send(client->socket, (BYTE*)&query, sizeof(Query))) != NOERROR)
	{
		errno_saveErrorMessage(err);
		printf("An error occured when sending data: (%d) %ws\n", err, errno_errMessage);
		return RESPONSE_NONE;
	}

	message.sender = client->user.id;
	message.receiver = userId;
	strcpy_s(message.text, MESSAGE_TEXT_LEN, text);
	time_t t = time(NULL);
	localtime_s(&message.timeSent, &t);

	if ((err = wsa_send(client->socket, (BYTE*)&message, sizeof(Message))) != NOERROR)
	{
		errno_saveErrorMessage(err);
		printf("An error occured when sending data: (%d) %ws\n", err, errno_errMessage);
		return RESPONSE_NONE;
	}

	if ((err = wsa_receive(client->socket, (BYTE*)&response, sizeof(Response))) != NOERROR)
	{
		errno_saveErrorMessage(err);
		printf("An error occured when receiving data: (%d) %ws\n", err, errno_errMessage);
		return RESPONSE_NONE;
	}

	switch (response.type)
	{
	case RESPONSE_YES:
		break;

	case RESPONSE_NO:
		printf("Server denied: %s\n", response.message);
		break;

	case RESPONSE_ERROR:
		printf("Error: %s\n", response.message);
		break;

	case RESPONSE_NONE:
		printf("Server didn't respond\n");
		break;

	default:
		printf("Unexpected response: %s\n", response.message);
		break;
	}
	return response.type;
}

BYTE client_query_getMessagesFromUser(Client* client, id userId, Message** messages, int* messageCount)
{
	errno_t err;
	Query query;
	Response response;
	query.type = QUERY_GETMESSAGESFROMUSER;
	query.dataSize = sizeof(id);
	if ((err = wsa_send(client->socket, (BYTE*)&query, sizeof(Query))) != NOERROR)
	{
		errno_saveErrorMessage(err);
		printf("An error occured when sending data: (%d) %ws\n", err, errno_errMessage);
		return RESPONSE_NONE;
	}

	if ((err = wsa_send(client->socket, (BYTE*)&userId, sizeof(id))) != NOERROR)
	{
		errno_saveErrorMessage(err);
		printf("An error occured when sending data: (%d) %ws\n", err, errno_errMessage);
		return RESPONSE_NONE;
	}

	if ((err = wsa_receive(client->socket, (BYTE*)&response, sizeof(Response))) != NOERROR)
	{
		errno_saveErrorMessage(err);
		printf("An error occured when receiving data: (%d) %ws\n", err, errno_errMessage);
		return RESPONSE_NONE;
	}

	switch (response.type)
	{
	case RESPONSE_YES:
		*messageCount = response.multiResponse.count;
		*messages = calloc(*messageCount, sizeof(Message));

		for (int i = 0; i < *messageCount; ++i)
		{
			if ((err = wsa_receive(client->socket, (BYTE*)(*messages + i), sizeof(Message))) != NOERROR)
			{
				errno_saveErrorMessage(err);
				printf("An error occured when receiving data: (%d) %ws\n", err, errno_errMessage);
				return RESPONSE_ERROR;
			}
		}
		break;

	case RESPONSE_NO:
		printf("Server denied: %s\n", response.message);
		break;

	case RESPONSE_ERROR:
		printf("Error: %s\n", response.message);
		break;

	case RESPONSE_NONE:
		printf("Server didn't respond\n");
		break;

	default:
		printf("Unexpected response: %s\n", response.message);
		break;
	}
	return response.type;
}

void client_dispose(Client* client)
{
	errno_t err;

	if ((err = wsa_closeSocket(client->socket)) == NOERROR)
	{
		printf("Socket 0x%04x was closed.\n", client->socket);
	}
	else
	{
		errno_saveErrorMessage(err);
		printf("An error occured when closing socket: (%d) %ws", err, errno_errMessage);
	}

	if ((err = wsa_cleanup(client->wsa)) == NOERROR)
	{
		printf("WSA cleaned up.\n");
	}
	else
	{
		errno_saveErrorMessage(err);
		printf("An error occured when cleaning WSA: (%d) %ws", err, errno_errMessage);
	}

	free(client);
}
