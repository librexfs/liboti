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

#include <string.h>
#include "otilib.h"
#include "protocol.h"

/* Doxygen format:*/
/************************** WriteDisplay() ****************************************************************//**
* Write text message to reader display.
*
* This command require that PCD LCD screen controlled by Host (by calling DoOutPortControl()). \n
* \note This function assumes character display (e.g. SATURN 6500) and does not work well on SATURN 6000 graphic display!

* @return	 OTI_STATUS				\n
			| Value					   |	Description																   |
			|--------------------------|-------------------------------------------------------------------------------|
			| OTI_OK				   | Success																	   |
			| OTI_DATA_LENGTH_OVERFLOW | line1 and\\or line2 is longer then 16 characters				 			   |
			| OTI_FAILED	    	   | Command execution failed on Saturn reader			 						   |
			| OTI_MEMORY_ERROR		   | Message buffer couldn't be allocated										   |
			| OTI_UNSUPPORTED		   | Command not supported by Saturn reader										   |

* See MapErrorToStatus() for extra error messages returned by ExchangeCmd()
*
* \ingroup LCD
********************************************************************************************************************/
OTI_STATUS WriteDisplay(char* line1, //!< [in] Line 1 to display
						char* line2, //!< [in] Line 2 to display
						PMSG_BUFF cmdWriteDisply, //!< [in] Buffer for build the command. the length of the buffer is 37
						PMSG_BUFF msgBuf) //!< [in] Buffer for building message
{
	OTI_STATUS ret;
	unsigned int i, characters_count = 0;
	RESP_MSG_STRUCT resp;
	cmdWriteDisply->Data[0] = 0xDF;
	cmdWriteDisply->Data[1] = 0x32;
	cmdWriteDisply->Data[2] = 0xFF /*LEN - REPLACED*/;
	cmdWriteDisply->Data[3] = 0x01;
	cmdWriteDisply->Data[4] = 0x80;

	// Tag use example: 
	//	DF 32 - Write display tag
	//	13	  - Length
	//	01    - Clear
	//	80    - Start from the begin of the line
	//	48 65 6C 6C 6F C0 57 6F 72 6C 64 21 - "Hello\nWorld!

	if (strlen(line1) > 16 || strlen(line2) > 16) // 16 characters length limit.
		return OTI_DATA_LENGTH_OVERFLOW;

	WORD msgBufLen = msgBuf->Max_Length;

	for (i = 0; i < strlen(line1); ++i, ++characters_count)
		cmdWriteDisply->Data[5 + i] = line1[i];

	cmdWriteDisply->Data[5 + (characters_count++)] = 0xC0;

	for (i = 0; i < strlen(line2); ++i, ++characters_count)
		cmdWriteDisply->Data[5 + characters_count] = line2[i];

	cmdWriteDisply->Data[2] = characters_count + 2;

	ret = ExchangeCmd(OP_DO, cmdWriteDisply->Data, 5 + characters_count, msgBuf->Data, &msgBufLen, &resp, SAT_DEFAULT_TIMEOUT);

	if (ret != OTI_OK)
	{
		LOG_DEBUG("WriteDisplay FAILED\r\n");
		return ret;
	}
	msgBuf->Actual_Length = msgBufLen;
	if (resp.succ == NULL)
	{
		if (resp.fail != NULL)
		{
			if (resp.failLen == 3)   // DF 32 ERR
				ret = MapErrorToStatus(resp.fail[2]);
			else
				ret = OTI_FAILED;

			return ret;
		}
		else
		{
			return OTI_UNSUPPORTED;
		}
	}

	return OTI_OK;
} // WriteDisplay()

// end
