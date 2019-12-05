#include <Windows.h>
#include <stdio.h>
#include "error_printer.h"

wchar_t errno_errMessage[ERR_MESSAGE_SIZE];

void errno_saveErrorMessage(errno_t err)
{
	FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		errno_errMessage, ERR_MESSAGE_SIZE, NULL);
}

void errno_printMessage()
{
	printf("%ws", errno_errMessage);
}