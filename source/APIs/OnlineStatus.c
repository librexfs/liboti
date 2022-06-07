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

#include <stdlib.h>
#include "protocol.h"
#include "frames.h"
#include "otilib.h"
#include "tlv.h"

/* Doxygen format:*/
/************************** OnlineStatus() *******************************************************************//**
* Tells the reader the status of the online transaction.
*
* This API is meant to be used when the Saturn Reader has returned from a PollEMV command 
* and the returned ReaderTransactionResult is “Online”. \n
* In that case, the reader displays a waiting message, starts a timer, and waits for an OnlineStatus message. \n
* If the reader does not receive such a message before the configured timeout is reached, it will stop waiting, 
* display a “communication error” message, and return to idle mode.

* @return	 OTI_STATUS				\n
			| Value					|	Description																	|
			|-----------------------|-------------------------------------------------------------------------------|
			| OTI_OK				| Function completed successfully				 								|
			| OTI_FAILED	    	| Command execution failed on Saturn reader			 							|
			| OTI_MEMORY_ERROR		| Message buffer couldn't be allocated											|
			| OTI_UNSUPPORTED		| Command not supported by Saturn reader										|

* See MapErrorToStatus() for extra error messages returned by ExchangeCmd()
*
* \ingroup EMV
********************************************************************************************************************/
OTI_STATUS OnlineStatus(ONLINE_STATUS status,
	PMSG_BUFF msgBuf) //!< [in] Buffer for building message
{
    RESP_MSG_STRUCT resp;
    OTI_STATUS ret;
    BYTE cmdBuf[] = {0xDF, 0x6A, 0x01, 0x00};
    
    WORD msgBufLen = msgBuf->Max_Length;

    /* Build 'Terminal Online Status' message */
    cmdBuf[3] = status;
    
    ret = ExchangeCmd(OP_DO, cmdBuf, 4, msgBuf->Data, &msgBufLen, &resp, SAT_DEFAULT_TIMEOUT);

    if(ret != OTI_OK)
    {
        LOG_DEBUG("ONLNSTAT CMD ERROR");
        return ret;
    }
    msgBuf->Actual_Length = msgBufLen;

    if(resp.succ == NULL)
    {
        if(resp.fail != NULL)
        {
            if(resp.failLen == 4)   // DF 6A 01 ERR
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
}// OnlineStatus()

OTI_STATUS WaitTimeExtention(PMSG_BUFF msgBuf) //!< [in] Buffer for building message
{
    RESP_MSG_STRUCT resp;
    OTI_STATUS ret;
    BYTE cmdBuf[] = { 0xDF, 0x81, 0x76, 0x00 };
    
    WORD msgBufLen = msgBuf->Max_Length;

    ret = ExchangeCmd(OP_DO, cmdBuf, 4, msgBuf->Data, &msgBufLen, &resp, SAT_DEFAULT_TIMEOUT);

    if (resp.succ == NULL)
    {
        if (resp.fail != NULL)
        {
            if (resp.failLen == 5)   // DF 81 76 01 ERR
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

    msgBuf->Actual_Length = msgBufLen;

    return OTI_OK;
}

// end

