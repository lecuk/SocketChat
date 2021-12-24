#pragma once

#include <Windows.h>
#include "message.h"

#define UPDATE_SIZE 128

#define UPDATE_NEWMESSAGE 0x01;
#define UPDATE_NEWCONVERSATION 0x02;

typedef union sUpdate
{
	BYTE data[UPDATE_SIZE];
	struct
	{
		BYTE type;
		union
		{
			struct
			{
				Message message;
			} newMessage;

			struct
			{
				id user;
			} wantsAConversation;
		};
	};
} Update;