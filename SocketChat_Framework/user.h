#pragma once

#include "id.h"

#define USER_NAME_LEN 20

#define USERSTATE_OFFLINE 0x00
#define USERSTATE_ONLINE 0x01
#define USERSTATE_UNKNOWN 0xFF

typedef struct sUser
{
	char state;
	id id;
	char name[USER_NAME_LEN];
	long messagesSent;
	long messagesReceived;
} User;