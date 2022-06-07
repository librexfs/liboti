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
#include <stdlib.h>
#include "protocol.h"
#include "otilib.h"


OTI_STATUS cmdPatternsControl(	BYTE **buf,
								WORD *bufLen,
								UI_PATTERN pattern)
{
	BYTE cmdLen;

	if(!buf || !bufLen)
		return OTI_MEMORY_ERROR;

	/* As of 2.12, only OP_DO is allowed */
	cmdLen = 0x04;	// DF 48 01 <PATTERN>

		/* Fixed minimum size needed for command */
		if(*bufLen < cmdLen)
			return OTI_BUFFER_OVERFLOW;

	**buf		= 0xDF;
	*(*buf + 1)	= 0x48;
	*(*buf + 2)	= 0x01;
	*(*buf + 3)	= pattern;


	/* Set final size of returned buffer */
	*bufLen = cmdLen;

	return OTI_OK;
} // cmdPatternsControl()


/* Doxygen format:*/
/************************** PatternsControl() **********************************************************************//**
* Enables the Host to activate one of the pre-defined LEDs and buzzer patterns of the Reader.
*
* This command requires setting the PCD to "Out Port controlled by Host"; otherwise, an out-of-scope error will be returned- 
* See SetOutPortControl() \n
* The patterns may look and sound a bit different, depending on the different PCD hardware and drivers.
* 
* @return	 OTI_STATUS				\n
			| Value					|	Description																	|
			|-----------------------|-------------------------------------------------------------------------------|
			| OTI_OK				| Function completed successfully				 								|
			| OTI_MEMORY_ERROR		| Message buffer couldn't be allocated											|
			| OTI_BUFFER_OVERFLOW	| Given buffer is not long enough												|

* See MapErrorToStatus() for extra error messages returned by ExchangeCmd()
*
* \ingroup UI
********************************************************************************************************************/
OTI_STATUS PatternsControl(UI_PATTERN pattern, //!< [in] UI Pattern
	PMSG_BUFF msgBuf) //!< [in] Buffer for building message
{
	RESP_MSG_STRUCT resp;
	OTI_STATUS ret = OTI_OK;
	BYTE  patternCmd[4];
	BYTE *patternCmdPtr = (BYTE *)patternCmd;
	WORD  patternCmdLen = 0x04;

	WORD msgBufLen = msgBuf->Max_Length;

	/* Build 'Patterns Control' message */
	ret = cmdPatternsControl(&patternCmdPtr, &patternCmdLen, pattern);

	if (ret != OTI_OK)
	{
		return ret;
	}

	ret = ExchangeCmd(OP_DO, patternCmd, patternCmdLen, msgBuf->Data, &msgBufLen, &resp, SAT_DEFAULT_TIMEOUT);

	if (ret != OTI_OK)
	{
		LOG_DEBUG("Patterns Control CMD ERROR");
		return ret;
	}

	if (resp.succ == NULL)
	{
		if (resp.fail != NULL)
		{
			if (resp.failLen == 4)   // DF 48 01 <ERR>
				ret = MapErrorToStatus(resp.fail[3]);
			else
				ret = OTI_FAILED;

			return ret;
		}
		else
		{
			return OTI_UNSUPPORTED;
		}
	}

	/* No response expected, no further parsing needed */

	return OTI_OK;

} // PatternsControl()

//end
