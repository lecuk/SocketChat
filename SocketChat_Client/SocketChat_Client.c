#include <stdio.h>
#include "client.h"
#include "WSA.h"
#include "pretty_console.h"
#include "color_support.h"
#include "error_printer.h"
#include "response.h"

Client* client;

const char* getInput(char* input, const size_t maxSize)
{
	if (maxSize < 2) return NULL;

	printf(C_FG_GREEN);
	do
	{
		printf("> ");
		fgets(input, maxSize, stdin); //i don't know why, but gets_s doesn't work
		fseek(stdin, 0, SEEK_END); //seeking stdin to prevent further reading if user entered more characters than maxSize
	} while (!*input || input[0] == '\n');
	char* newLine;
	if (newLine = strrchr(input, '\n'))
	{
		*newLine = 0;
	}
	printf(C_RESET);
	return input;
}

int main()
{
	char userInput[64];
	if (EnableColorSupport(TRUE))
	{
		printf(C_BG_PURPLE C_FG_ORANGE "Color support enabled.\n" C_RESET);
	}
	else
	{
		printf("Color support disabled.\n");
	}

	client = client_init();

	BYTE a1, a2, a3, a4;
	unsigned short port;

	printf("Enter server address and port:\n");
	while (1)
	{
		getInput(userInput, 64);
		if (sscanf_s(userInput, "%hhu.%hhu.%hhu.%hhu:%hu", &a1, &a2, &a3, &a4, &port) != 5)
		{
			printf(C_FG_YELLOW "Invalid input. Try again. (format: " C_FG_ORANGE "ip1.ip2.ip3.ip4:port" C_FG_YELLOW ")\n");
			continue;
		}

		printf(C_RESET "Connecting...\n");
		if (!client_connect(client, makeAddress(a1, a2, a3, a4, port)))
		{
			printf(C_FG_YELLOW "Can't connect to server!\n" C_RESET);
			continue;
		}

		break;
	}

	printf("Enter your name:\n");
	while (1)
	{
		getInput(userInput, 64);

		printf(C_RESET "Entering...\n");
		if (!client_enterConnectedServer(client, userInput))
		{
			printf(C_FG_YELLOW "Can't enter server!\n" C_RESET);
			continue;
		}

		break;
	}

	printf("Welcome to server, " C_FG_YELLOW "%s" C_RESET ". Your Id: " C_FG_YELLOW "%d\n" C_RESET, client->user.name, client->user.id);

	printf("Enter your command:\n");
	printf("  " C_FG_AQUA "online" C_RESET " - prints how many users are on server\n");
	printf("  " C_FG_AQUA "list" C_RESET " - list all users\n");
	printf("  " C_FG_AQUA "info" C_FG_YELLOW " id" C_RESET " - prints info about user\n");
	printf("  " C_FG_AQUA "message" C_FG_YELLOW " id" C_RESET " - sends a message to user\n");
	printf("  " C_FG_AQUA "inbox" C_FG_YELLOW " id" C_RESET " - gets messages from user\n");
	printf("  " C_FG_AQUA "exit" C_RESET " - exit from program\n");
	while (TRUE)
	{
		getInput(userInput, 64);

		if (!strcmp(userInput, "online"))
		{
			int online, maxOnline;
			if (client_query_howManyUsers(client, &online, &maxOnline) == RESPONSE_OK)
			{
				printf("There are currently %d/%d users online.\n", online, maxOnline);
			}
		}
		else if (strstr(userInput, "info") == userInput)
		{
			id userId;
			User user;

			if (sscanf_s(userInput, "info %hu", &userId) != 1)
			{
				printf(C_FG_ORANGE "Invalid arguments.\n" C_RESET);
				continue;
			}

			if (client_query_getUserById(client, userId, &user) == RESPONSE_YES)
			{
				printf("Found user " C_FG_YELLOW "%s" C_RESET " #%hu.\n", user.name, user.id);
				printf("Status: %s.\n", 
					(user.state == USERSTATE_ONLINE) ? C_FG_GREEN "ONLINE" C_RESET : 
					(user.state == USERSTATE_OFFLINE) ? C_FG_ORANGE "OFFLINE" C_RESET : 
					C_FG_GRAY "UNKNOWN" C_RESET);
				printf("Messages: sent %d, received %d.\n", user.messagesSent, user.messagesReceived);
			}
		}
		else if (!strcmp(userInput, "list"))
		{
			User* users;
			int userCount;
			if (client_query_listUsers(client, &users, &userCount) == RESPONSE_OK)
			{
				printf("Users online: ");
				if (userCount > 0) printf("%s #%d", users[0].name, users[0].id);
				for (int i = 1; i < userCount; ++i)
				{
					printf(", %s #%d", users[i].name, users[i].id);
				}
				printf("\n");
				free(users);
			}
		}
		else if (strstr(userInput, "message") == userInput)
		{
			id userId;
			if (sscanf_s(userInput, "message %hu", &userId) != 1)
			{
				printf(C_FG_ORANGE "Invalid arguments.\n" C_RESET);
				continue;
			}

			printf("Enter your message:\n");
			getInput(userInput, 64);
			client_query_sendMessage(client, userId, userInput);
		}
		else if (strstr(userInput, "inbox") == userInput)
		{
			id userId;
			if (sscanf_s(userInput, "inbox %hu", &userId) != 1)
			{
				printf(C_FG_ORANGE "Invalid arguments.\n" C_RESET);
				continue;
			}

			Message* messages;
			int messageCount;
			if (client_query_getMessagesFromUser(client, userId, &messages, &messageCount) == RESPONSE_YES)
			{
				for (int i = 0; i < messageCount; ++i)
				{
					char timeSentStr[64];
					strftime(timeSentStr, 64, "%d %b %Y, %H:%M:%S", &(messages[i].timeSent));
					printf(C_FG_GRAY "[%s]: %s\n" C_RESET, timeSentStr, messages[i].text);
				}
				free(messages);
			}
		}
		else if (!strcmp(userInput, "exit")) break;
		else continue;
	}

	client_query_disconnect(client);
	client_dispose(client);

	return 0;
}