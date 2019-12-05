#include "client.h"
#include "WSA.h"
#include <WinSock2.h>

Client* client_init(const char* name)
{
	Client* client = malloc(sizeof(Client));
	return client;
}

void client_dispose(Client* client)
{
	free(client);
}
