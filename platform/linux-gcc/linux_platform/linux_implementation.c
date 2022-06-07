#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "platform_utils.h"

static oti_ext_trace_fn ext_log = NULL;
static void* user_data = NULL;

void OtiLibSetLogger( oti_ext_trace_fn fn, void* data ) {
    ext_log = fn;
    user_data = data;
}

void SystemSleep(int ms)
{
	usleep(ms * 1000);
}

void FormatDevicePath(BYTE *buf, BYTE *path)
{
	int len = strlen((char*)path);
    memcpy(buf, path, len);
    buf[len] = '\0';
}

void *MALLOC(int size)
{
	return malloc(size);
}

void FREE(void *ptr)
{
	free(ptr);
}

void lWrite(char *msg)
{
    printf("%s", msg);
}

void Sat_Port_Log(SAT_LOG_LEVEL level, char *msg)
{
    if( ext_log ) {
        ext_log( msg, user_data );
    } else {
        printf("%s", msg);
    }
}

void Sat_Port_Log_Buf(SAT_LOG_LEVEL level, const BYTE* py_Buf, WORD w_Length)
{
	int i;
	for(i=0; i < w_Length; ++i)
		printf("%02X ", py_Buf[i]);

	printf("\r\n");
}

static BOOLEAN isFirstAccess = TRUE;
static pthread_mutex_t mutex;
static pthread_mutexattr_t  attr;

static void SAT_Mutex_Init_If_Required()
{
	if (isFirstAccess)
	{
		pthread_mutexattr_init (&attr);
		pthread_mutex_init(&mutex, &attr);
	}
}
BOOLEAN SAT_Mutex_Acquire(DWORD u32_timeout_ms)
{
	SAT_Mutex_Init_If_Required();
	struct timespec timeout;
	timeout.tv_sec=0;
	timeout.tv_nsec= (u32_timeout_ms * 1000000); // Microseconds to miliseconds conversion.
	if (pthread_mutex_timedlock(&mutex, &timeout))
		return FALSE;
	else
		return TRUE;
}

BOOLEAN SAT_Mutex_Release(void)
{
	SAT_Mutex_Init_If_Required();
	if (pthread_mutex_lock(&mutex))
		return FALSE;
	else
		return TRUE;
}
