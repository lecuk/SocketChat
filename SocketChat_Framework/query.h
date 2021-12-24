#pragma once

#include <Windows.h>
#include "id.h"
#include "message.h"
#include "user.h"

// Query format:
// QUERY_CONNECT|USER_NAME_LEN + (char* name)
//
// Expected responses:
// RESPONSE_OK|"ok"|0
// RESPONSE_ERROR|"error"|0
#define QUERY_CONNECT 0x01

// Query format:
// QUERY_DISCONNECT|0
//
// Expected responses:
// RESPONSE_OK|"ok"|0
// RESPONSE_ERROR|"error"|0
#define QUERY_DISCONNECT 0x02

// Query format:
// QUERY_HOWMANYUSERS|0
//
// Expected responses:
// RESPONSE_OK|"ok"|onlineUsers,maxUsers|0
// RESPONSE_ERROR|"error"|0
#define QUERY_HOWMANYUSERS 0x03

// Query format:
// QUERY_LISTUSERS|0
//
// Expected responses:
// RESPONSE_OK: message = "ok" | singleResponse = 0 | multiResponse = {count = userCount, size = sizeof(User)}
// RESPONSE_ERROR|"error"|0
#define QUERY_LISTUSERS 0x04

#define QUERY_GETUSERINFO 0x05
// 0x05|sizeof(id)|id

#define QUERY_SENDMESSAGE 0x06
// 0x06|sizeof(Message)|Message

#define QUERY_GETMESSAGESFROMUSER 0x07
// 0x07|sizeof(id)|id

#define QUERY_GETUSERBYNAME 0x08
// 0x08|USER_NAME_LEN|id

#define QUERY_GETUSERBYID 0x09
// 0x09|sizeof(id)|id

typedef struct sQuery
{
	BYTE type;
	size_t dataSize;
} Query;