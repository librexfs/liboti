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
#include "otilib.h"
#include "tlv.h"
#include "protocol.h"
#include "config.h"

/* Doxygen format:*/
/************************** Ping() *******************************************************************************//**
* Check if Saturn reader is alive.
*
* Sends ReadSerialNumber command and waits for response from Saturn
*
* @return	 OTI_STATUS				\n
			| Value					|	Description																	|
			|-----------------------|-------------------------------------------------------------------------------|
			| OTI_OK				| Function completed successfully				 								|
			| OTI_MEMORY_ERROR		| Message buffer couldn't be allocated											|
			| OTI_FAILED	    	| Command execution failed on Saturn reader			 							|

* See MapErrorToStatus() for extra error messages returned by ExchangeCmd()
*
* \ingroup GENERAL
********************************************************************************************************************/
OTI_STATUS Ping(PMSG_BUFF msgBuf) //!< [in] Buffer for building message
{
	RESP_MSG_STRUCT resp;
	OTI_STATUS status = OTI_OK;
	WORD msgBufLen = msgBuf->Max_Length;

	status = ExchangeCmd(OP_GET, (BYTE *)"\xDF\x4D\x00", 3, msgBuf->Data, &msgBufLen, &resp, SAT_DEFAULT_TIMEOUT);
	msgBuf->Actual_Length = msgBufLen;
	return status;
}

// end
