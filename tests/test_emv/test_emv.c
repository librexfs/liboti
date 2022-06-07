/*
OTI HostPCD Protocol C Library

**********************************************************************
**        Copyright (c) 2008-2015 On Track Innovations Ltd.         **
** This computer program includes confidential, proprietary         **
** information and is a trade secret of On Track Innovations Ltd.   **
** All use, disclosure, and/or reproduction is prohibited           **
** unless expressly authorized in writing.                          **
**                    All rights reserved.                          **
**********************************************************************

Description:    Test application that calls some of the Library's functions.
Author:         On Track Innovations Ltd.
Version:        4.0.00

Notes:          If the library was compiled with -DDEBUG_MODE, this will  display some test results.
Otherwise, a mere informational message should be displayed.

*/

#include <stdio.h>
#include <string.h>

#include "otilib.h"
#include "protocol.h"
#include "serial.h"

#include "platform_utils.h"
#include "polling_manager.h"

BOOLEAN OnlineRequest_Handler(TLV_MSG *fc, TLV_MSG *incomingE8)
{
	SystemSleep(4000);
	return TRUE;
}

void OfflineRequest_Handler(TLV_MSG *fc)
{
	
}

BOOLEAN Do()
{
	ACT_EMV_STRUCT act_trans;

	TLV_MSG resp;
	BYTE respBuffer[MAX_BUFFER_SIZE];
	resp.Data = respBuffer;
	resp.CurrentLocation = 0;
	resp.Length = MAX_BUFFER_SIZE;

	// Init:
	InitTransaction(OnlineRequest_Handler, OfflineRequest_Handler);

	// ********************************************************************************************************************************
	// Building the command for the PCD:
	act_trans.properties = 0; // reset all ENABLE flags.

	// *** Mandatory tags:
	// Amount Authorized- 100.99
	memcpy(act_trans.amt_auth, "\x00\x00\x00\x00\x10\x00", 6);
	// Currency Code
	memcpy(act_trans.currency_code, "\x08\x40", 2);
	// Date- 08/08/18
	memcpy(act_trans.date, "\x03\x03\x13", 3);


	// *** Optional tags:
	// Time- 01:01:01
	memcpy(act_trans.trans_time, "\x01\x01\x01", 3);
	act_trans.properties |= EMV_TRANS_TIME;
	// Purchase (of goods or services)
	act_trans.trans_type = 0x00;
	act_trans.properties |= EMV_TRANS_TYPE;
	// Activating DUKPT (by un-comment the following two lines):
	//memcpy(act_trans.dukpt_key, "\x01", 1); 
	//act_trans.properties |= EMV_TRANS_DUKPT;
	// ********************************************************************************************************************************

	return DoTransaction(&act_trans, &resp);
}

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		LOG_TRACE("Error! Invalid arguments.\r\n");
		LOG_TRACE("Using example: %s COM_PORT\r\n", argv[0]);
		return 1;
	}

	if (SetReaderPort((BYTE *)argv[1]) != OTI_OK)
	{
		LOG_TRACE("Opening reader comm failed!");
		return 2;
	}
	while (1)
	{
		if (!Do()) // Start payment transaction.
			break; // Last transaction failed.

		LOG_OUTPUT("**********************************************************************\r\n");
		SystemSleep(2000);
	}

	return 1;
}
