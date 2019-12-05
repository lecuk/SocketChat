#pragma once
#include "message.h"
#include "llist.h"
#include "user.h"

#define CHAT_MAX_USERS 4
#define CHAT_NAME_LEN 20

typedef struct sChat
{
	id id;
	const char name[CHAT_NAME_LEN];
	LinkedList* users;
	LinkedList* messages;
} Chat;
