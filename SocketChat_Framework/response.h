#pragma once

#include <Windows.h>
#include "message.h"
#include "user.h"

#define RESPONSE_MESSAGE_LEN 64
#define RESPONSE_DATA_LEN 32

#define RESPONSE_NONE	0x00
#define RESPONSE_OK		0x01
#define RESPONSE_YES	0x02
#define RESPONSE_NO		0x03
#define RESPONSE_ERROR	0x04

//Response can come with a single recv() call, then its data will be in singleResponseData.
//But if data sent by server is an array with size N it will be divided into N blocks
typedef struct sResponse
{
	BYTE type;
	char message[RESPONSE_MESSAGE_LEN];

	union
	{
		BYTE data[RESPONSE_DATA_LEN];

		struct
		{
			int online;
			int maxOnline;
		} userCount;

		User user;
		id id;
	} singleResponse;

	struct
	{
		size_t dataSize;
		size_t count;

	} multiResponse;

} Response;