#include <stdio.h>
#include "client.h"
#include "WSA.h"
#include "pretty_console.h"
#include "color_support.h"

Client* client;

int main()
{
	if (EnableColorSupport(TRUE))
	{
		printf(C_BG_PURPLE C_FG_ORANGE "Color support enabled.\n" C_RESET);
	}

	char command[20];
	while (1)
	{
		printf("Enter your command:\n");
		printf("  " C_FG_AQUA "chat" C_RESET " - opens a chat\n");
		printf("  " C_FG_AQUA "list" C_RESET " - list all users\n");
		printf("  " C_FG_AQUA "info" C_RESET " - prints info about user\n");
		printf("  " C_FG_AQUA "message" C_RESET " - sends a message to user\n");
		printf("  " C_FG_AQUA "exit" C_RESET " - exit from program\n");
		scanf_s("%s", command, 20);

		     if (!strcmp(command, "chat"));
		else if (!strcmp(command, "info"));
		else if (!strcmp(command, "message"));
		else if (!strcmp(command, "exit")) break;
		else continue;
	}

	client_dispose(client);

	return 0;
}