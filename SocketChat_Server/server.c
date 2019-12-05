#include "server.h"
#include <stdio.h>
#include "error_printer.h"
#include <WS2tcpip.h>
#include <WinSock2.h>

Server* server_init(const char* service)
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
		printf("An error occured when loading WSA: (%d) %ws", err, errno_errMessage);
	}

	if ((err = wsa_getCurrentAddressInfo(&server->addrinfo, service)) == NOERROR)
	{
		server->address = (SOCKADDR_IN*)server->addrinfo->ai_addr;
		inet_ntop(AF_INET, server->address, server->addressString, 64);
		printf("Server address: %s\n", server->addressString);
		/*
		printf("Server address loaded: %d.%d.%d.%d:%d\n",
			server->address->sin_addr.S_un.S_un_b.s_b1,
			server->address->sin_addr.S_un.S_un_b.s_b2,
			server->address->sin_addr.S_un.S_un_b.s_b3,
			server->address->sin_addr.S_un.S_un_b.s_b4,
			server->address->sin_port); 
		*/
	}
	else
	{
		errno_saveErrorMessage(err);
		printf("An error occured when loading address: (%d) %ws", err, errno_errMessage);
	}

	if ((err = wsa_openSocket(&server->socket)) == NOERROR)
	{
		printf("Socket with handle 0x%04x was opened.\n", server->socket);
	}
	else
	{
		errno_saveErrorMessage(err);
		printf("An error occured when loading address: (%d) %ws", err, errno_errMessage);
	}

	if ((err = wsa_bind(server->socket, *server->address)) == NOERROR)
	{
		printf("Socket 0x%04x was bound to address %s.\n", server->socket, server->addressString);
	}
	else
	{
		errno_saveErrorMessage(err);
		printf("An error occured when binding socket: (%d) %ws", err, errno_errMessage);
	}

	server->clients = llist_create(NULL, FALSE, sizeof(ClientInfo*));

	return server;
}

void server_run(Server* server)
{

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
		printf("An error occured when listening to socket: (%d) %ws", err, errno_errMessage);
	}
}

void server_dispose(Server* server)
{
	errno_t err;

	if (server)
	{
		if ((err = wsa_closeSocket(server->socket)) == NOERROR)
		{
			printf("Socket 0x%04x was closed.\n", server->socket);
		}
		else
		{
			errno_saveErrorMessage(err);
			printf("An error occured when closing socket: (%d) %ws", err, errno_errMessage);
		}

		if ((err = wsa_freeAddressInfo(server->addrinfo)) == NOERROR)
		{
			printf("Address info of server was disposed.\n");
		}
		else
		{
			errno_saveErrorMessage(err);
			printf("An error occured when freeing address info: (%d) %ws", err, errno_errMessage);
		}

		llist_dispose(server->clients);
	}

	if ((err = wsa_cleanup(server->wsa)) == NOERROR)
	{
		printf("WSA cleaned up.\n");
	}
	else
	{
		errno_saveErrorMessage(err);
		printf("An error occured when cleaning WSA: (%d) %ws", err, errno_errMessage);
	}

	free(server);
}
