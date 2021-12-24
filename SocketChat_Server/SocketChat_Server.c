#include <stdio.h>
#include "WSA.h"
#include <conio.h>
#include "llist.h"
#include "server.h"

int main()
{
	BYTE a1, a2, a3, a4;
	unsigned short port;

	printf("Enter server address and port in format ip1.ip2.ip3.ip4:port\n> ");
	while (scanf_s("%hhu.%hhu.%hhu.%hhu:%hu", &a1, &a2, &a3, &a4, &port) != 5)
	{
		printf("Invalid address!\n> ");
		fseek(stdin, 0, SEEK_END);
		continue;
	}
	Server* server = server_init(makeAddress(a1, a2, a3, a4, port));

	if (!server) return -1;

	printf("Press ENTER to close the server.\n");
	server_listen(server);

	getchar();
	getchar();

	server_dispose(server);

	return 0;
}