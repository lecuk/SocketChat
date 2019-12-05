#include "user.h"
#include <stdlib.h>
#include <string.h>

User* user_create(id id, const char* name)
{
	User* user = malloc(sizeof(User));
	user->state = USERSTATE_ONLINE;
	user->id = id;
	strcpy_s(user->name, USER_NAME_LEN, name);
	return user;
}

void user_dispose(User * user)
{
	free(user);
}
