#pragma once

#include "id.h"
#include "date_time.h"

#define MESSAGE_TEXT_LEN 100

typedef struct sMessage
{
	short sender;
	short receiver;
	char text[MESSAGE_TEXT_LEN];
	struct tm timeSent;
} Message;
