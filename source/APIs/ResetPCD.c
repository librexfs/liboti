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
/************************** ResetPCD() ****************************************************************************//**
* Send a ResetPCD command to the reader.
*
* This API reboots the Saturn reader. \n
* This reset is driven by stopping the watchdog handling, and takes few hundred milliseconds. \n
* It also loads the PCD’s Default values – such as a call to the LoadDefaultParams() API.
* 
* @return	 OTI_STATUS				\n
			| Value					|	Description																	|
			|-----------------------|-------------------------------------------------------------------------------|
			| OTI_OK				| Function completed successfully				 								|
			| OTI_MEMORY_ERROR		| Message buffer couldn't be allocated											|
			| OTI_FAILED	    	| Command execution failed on Saturn reader			 							|
			| OTI_UNSUPPORTED		| Command not supported by Saturn reader										|

* See MapErrorToStatus() for extra error messages returned by ExchangeCmd()
*
* \ingroup GENERAL
********************************************************************************************************************/
OTI_STATUS ResetPCD(PMSG_BUFF msgBuf) //!< [in] Buffer for building message

{
 	  RESP_MSG_STRUCT resp;
    OTI_STATUS ret;
    WORD msgBufLen = msgBuf->Max_Length;

    ret = ExchangeCmd(OP_DO, (BYTE *)"\xDF\x19\x00", 3, msgBuf->Data, &msgBufLen, &resp, 10000);

    if(ret != OTI_OK)
    {
        LOG_DEBUG("ResetPCD CMD ERROR");
        return ret;
    }
    msgBuf->Actual_Length = msgBufLen;

    if(resp.succ == NULL)
    {
        if(resp.fail != NULL)
        {
            if(resp.failLen == 4)   // DF 19 01 ERR
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

    return OTI_OK;
}
