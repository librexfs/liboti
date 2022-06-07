#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include "..\..\..\include\platform-dependent\platform_utils.h"

#define snprintf _snprintf

void SystemSleep(int ms)
{
	Sleep(ms);
}

void FormatDevicePath(BYTE *buf, BYTE *path)
{
	int len = strlen(path);
	// The string "\\\\.\\" must be the prefix of the COM port because without this, there is no support for
	// COM ports that are greater then 9. Read more: http://support.microsoft.com/KB/115831
	memcpy(buf, "\\\\.\\", 4);
	memcpy(buf + 4, path, len);
	buf[4 + len] = '\0';
}

void lWrite(char *msg)
{
	printf("%s", msg);
}


void *MALLOC(int size)
{
	return malloc(size);
}

void FREE(void *ptr)
{
	free(ptr);
}
HANDLE mutex = NULL;

static void SAT_Mutex_Init_If_Required()
{
	if (mutex == NULL)
		mutex = CreateMutex(NULL, FALSE, NULL);
}

BOOLEAN SAT_Mutex_Acquire(DWORD u32_timeout_ms)
{
	SAT_Mutex_Init_If_Required();
	if (WaitForSingleObject(mutex, u32_timeout_ms) == 0)
		return TRUE;
	else
		return FALSE;
}

BOOLEAN SAT_Mutex_Release(void)
{
	SAT_Mutex_Init_If_Required();
	return ReleaseMutex(mutex);
}

void Sat_Port_Log(SAT_LOG_LEVEL level, char *msg)
{
	printf("%s", msg);
}

void Sat_Port_Log_Buf(SAT_LOG_LEVEL level, const BYTE* py_Buf, WORD w_Length)
{
	int i;
	for (i = 0; i < w_Length; ++i)
		printf("%02X ", py_Buf[i]);
	printf("\r\n");
}
