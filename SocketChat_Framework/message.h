#pragma once

#include "date_time.h"

#define MESSAGE_SIZE 100

typedef struct sMessage
{
	short sender;
	short receiver;
	char text[MESSAGE_SIZE];
	struct tm* timeSent;
} Message;

struct sMessage* message_create(short sender, short receiver, const char* text);
