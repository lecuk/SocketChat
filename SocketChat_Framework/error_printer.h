#pragma once

#define ERR_MESSAGE_SIZE (256)
extern wchar_t errno_errMessage[ERR_MESSAGE_SIZE];

void errno_saveErrorMessage(errno_t err);
void errno_printMessage();