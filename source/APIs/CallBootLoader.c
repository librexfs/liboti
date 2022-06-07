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
#include "protocol.h"

/* Doxygen format:*/
/************************** CallBootLoader() **********************************************************//**
* Calls the Saturn's bootloader section.
*
* This is a direct call for the Boot Loader program of the PCD (IAP - In Application Programming). \n
* It is mainly used from the Main FW to call operation of the Boot Loader (Change the used code sector). \n
* In case the current state is already in the Boot Loader the PCD will maintain its state.
*
* @return	 OTI_STATUS				\n
			| Value					|	Description																	|
			|-----------------------|-------------------------------------------------------------------------------|
			| OTI_OK				| Function completed successfully, resp is filled 								|
			| OTI_FAILED	    	| Command execution failed on Saturn reader			 							|
			| OTI_UNSUPPORTED		| Command not supported by Saturn reader										|
			| OTI_MEMORY_ERROR		| Message buffer couldn't be allocated											|

* See MapErrorToStatus() for extra error messages returned by ExchangeCmd()
* 
* \ingroup INTERNAL
********************************************************************************************************/
OTI_STATUS CallBootLoader(PMSG_BUFF msgBuf) //!< [in] Buffer for building message)	//!< [in] Buffer length for building message. If msgBufLen = 0 then method return the length or the buffer message and return with OTI_STATUS = OTI_BUFFER_LENGTH and without execute the message. 
{
    RESP_MSG_STRUCT resp;
    OTI_STATUS 		ret;

    WORD msgBufLen = msgBuf->Max_Length;


    ret = ExchangeCmd(OP_DO, (BYTE *)"\xDF\x40\x00", 3, msgBuf->Data, &msgBufLen, &resp, SAT_DEFAULT_TIMEOUT);

    if(ret != OTI_OK)
    {
        LOG_DEBUG("CallBootLoader CMD ERROR");
        return ret;
    }
    msgBuf->Actual_Length = msgBufLen;
    if(resp.succ == NULL)
    {
        if(resp.fail != NULL)
        {
            if(resp.failLen == 4)   // DF 40 01 ERR
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
} //CallBootLoader()

// end
