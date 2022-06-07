/********************************************************************************************************************
OTI HostPCD Protocol C Library

**********************************************************************
**        Copyright (c) 2008-2015 On Track Innovations Ltd.         **
** This computer program includes confidential, proprietary         **
** information and is a trade secret of On Track Innovations Ltd.   **
** All use, disclosure, and/or reproduction is prohibited           **
** unless expressly authorized in writing.                          **
**                    All rights reserved.                          **
**********************************************************************

PURPOSE:     This file contains a library function.

PROGRAMMER:  On Track Innovations Ltd.

REVISIONS:   4.0.00
*********************************************************************************************************************
*/
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "otilib.h"
#include "platform_utils.h"

#define MAX_MSG_SIZE 256

#ifdef OTI_LIB_LOG_ENABLED
#define TRACE_ENABLED
#define DEBUG_ENABLED
#define OUTPUT_ENABLED
#endif

void hWrite(log_level src, const char* msg, va_list args)
{
	static char MSG_BUFFER[MAX_MSG_SIZE];
	static char PREFIX_BUFFER[MAX_MSG_SIZE];
	SAT_LOG_LEVEL sat_Log_Level;
	
	switch (src)
	{
	case TRACE:
		strcpy(PREFIX_BUFFER, "TRACE: ");
		strcpy(PREFIX_BUFFER + 7, msg);
		sat_Log_Level = SAT_LOG_LEVEL_TRACE;
		break;
	case DEBUG:
		strcpy(PREFIX_BUFFER, "DEBUG: ");
		strcpy(PREFIX_BUFFER + 7, msg);
		sat_Log_Level = SAT_LOG_LEVEL_DEBUG;
		break;
	case OUTPUT:
		strcpy(PREFIX_BUFFER, msg);
		sat_Log_Level = SAT_LOG_LEVEL_INFO;
		break;
	default:
		return;
	}
	vsnprintf(MSG_BUFFER, MAX_MSG_SIZE, PREFIX_BUFFER, args);
	Sat_Port_Log(sat_Log_Level, MSG_BUFFER);
	va_end(args);
}


void LOG_TRACE(char *msg, ...)
{
#ifdef TRACE_ENABLED
	va_list args;
	va_start(args, msg);
	hWrite(TRACE, msg, args);
	va_end(args);
#endif
}

void LOG_DEBUG(char *msg, ...)
{
#ifdef DEBUG_ENABLED
	va_list args;
	va_start(args, msg);
	hWrite(DEBUG, msg, args);
	va_end(args);
#endif
}

void LOG_OUTPUT(char *msg, ...)
{
#ifdef OUTPUT_ENABLED
	va_list args;
	va_start(args, msg);
	hWrite(OUTPUT, msg, args);
	va_end(args);
	#endif
}

void LOG_OUTPUT_BUF(BYTE* py_buf, WORD w_Length)
{
#ifdef OUTPUT_ENABLED
	Sat_Port_Log_Buf(OUTPUT, py_buf, w_Length);
#endif
}
