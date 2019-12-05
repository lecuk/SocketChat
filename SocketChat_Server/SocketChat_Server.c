#include <stdio.h>
#include "WSA.h"
#include <conio.h>
#include "llist.h"
#include "server.h"

int main()
{
	Server* server = server_init("27015");

	printf("Press ENTER to close the server.\n");

	getchar();

	server_dispose(server);

	return 0;
}