#define _WIN32_WINNT 0x0600

#include "color_support.h"

#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif

typedef NTSTATUS(WINAPI*RtlGetVersionPtr)(PRTL_OSVERSIONINFOW);

/**
 * Check if STD_OUTPUT_HANDLE or STD_ERROR_HANDLE is redirected to a file.
 */
BOOL IsRedirectedToFile(DWORD stdHandle) 
{
	BOOL result = FALSE;

	HANDLE hStd = GetStdHandle(stdHandle);
	if (hStd != INVALID_HANDLE_VALUE) {
		if (GetFinalPathNameByHandle(hStd, NULL, 0, 0) != 0 || GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
			result = TRUE;
		}
	}

	return result;
}

/**
 * Check if the current console supports ANSI colors.
 */
BOOL HaveColorSupport()
{
	const DWORD MINV_MAJOR = 10, MINV_MINOR = 0, MINV_BUILD = 10586;
	int result = FALSE;
	HMODULE hMod = GetModuleHandleW(L"ntdll.dll");
	if (hMod) 
	{
		RtlGetVersionPtr fn = (RtlGetVersionPtr)GetProcAddress(hMod, "RtlGetVersion");
		if (fn != NULL) 
		{
			RTL_OSVERSIONINFOW rovi = { 0 };
			rovi.dwOSVersionInfoSize = sizeof(rovi);
			return (fn(&rovi) == 0) && 
				(rovi.dwMajorVersion > MINV_MAJOR) || 
				(rovi.dwMajorVersion == MINV_MAJOR) && 
				(rovi.dwMinorVersion > MINV_MINOR) || 
				(rovi.dwMinorVersion == MINV_MINOR) && 
				(rovi.dwBuildNumber >= MINV_BUILD);
		}
	}
	return 0;
}

BOOL ColorSupportEnabled() 
{
	BOOL result = FALSE;
	if (HaveColorSupport()) {
		HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
		if (hStdOut != INVALID_HANDLE_VALUE) {
			DWORD mode;
			if (GetConsoleMode(hStdOut, &mode)) {
				if (mode & ENABLE_VIRTUAL_TERMINAL_PROCESSING) {
					result = TRUE;
				}
			}
		}
	}

	return result;
}

/**
 * Enable/disable ANSI colors support for the current console.
 *
 * Returns TRUE if operation succeeded, FALSE otherwise.
 */
BOOL EnableColorSupport(BOOL enabled)
{
	BOOL result = FALSE;
	if (HaveColorSupport()) {
		HANDLE hStdOut;
		hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
		if (hStdOut != INVALID_HANDLE_VALUE) {
			DWORD mode;
			if (GetConsoleMode(hStdOut, &mode)) {
				if (((mode & ENABLE_VIRTUAL_TERMINAL_PROCESSING) ? 1 : 0) == (enabled ? 1 : 0)) {
					result = TRUE;
				}
				else {
					if (enabled) {
						mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
					}
					else {
						mode &= ~ENABLE_VIRTUAL_TERMINAL_PROCESSING;
					}
					if (SetConsoleMode(hStdOut, mode)) {
						result = TRUE;
					}
				}
			}
		}
	}

	return result;
}

LPSTR formatOkKo(LPSTR buffer, BOOL cond, LPCSTR ok, LPCSTR ko) 
{
	BOOL color = ColorSupportEnabled();
	buffer[0] = '\0';
	if (color) {
		strcat(buffer, cond ? "\x1b[92m" : "\x1b[91m");
	}
	strcat(buffer, cond ? ok : ko);
	if (color) {
		strcat(buffer, "\x1b[0m");
	}
	return buffer;
}