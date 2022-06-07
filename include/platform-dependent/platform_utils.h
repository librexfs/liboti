#ifndef PLATFORM_UTILS_H_
#define PLATFORM_UTILS_H_

#include "oti_types.h"

typedef enum 
{
	SAT_LOG_LEVEL__Undefined=0,
	SAT_LOG_LEVEL_ERROR,
	SAT_LOG_LEVEL_WARNING,
	SAT_LOG_LEVEL_INFO,
	SAT_LOG_LEVEL_DEBUG,
	SAT_LOG_LEVEL_TRACE,
	SAT_LOG_LEVEL__LAST,
} SAT_LOG_LEVEL;

#ifdef  __cplusplus
extern "C" {
#endif

void SystemSleep(int ms);

void FormatDevicePath(BYTE *buf, BYTE *path);

void lWrite(char *msg);
void Sat_Port_Log(SAT_LOG_LEVEL level, char *msg);
void Sat_Port_Log_Buf(SAT_LOG_LEVEL level, const BYTE* py_Buf, WORD w_Length);

BOOLEAN SAT_Mutex_Acquire(DWORD u32_timeout_ms);
BOOLEAN SAT_Mutex_Release(void);

/* Setup external logger */
typedef void ( *oti_ext_trace_fn )( char* buffer, void* user_data );
void OtiLibSetLogger( oti_ext_trace_fn trace_fn, void* data );

#ifdef  __cplusplus
}
#endif

#endif
