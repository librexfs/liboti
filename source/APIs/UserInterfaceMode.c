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
/************************** StoreUIMode() *************************************************************************//**
* Change the Reader User Interface mode.
*
* This API sets the Saturn Reader’s default user interface mode. \n
* 
* @return	 OTI_STATUS				\n
			| Value					|	Description																	|
			|-----------------------|-------------------------------------------------------------------------------|
			| OTI_OK				| Function completed successfully.				 								|
			| OTI_FAILED	    	| Command execution failed on Saturn reader			 							|
			| OTI_UNSUPPORTED		| Command not supported by Saturn reader										|
			| OTI_MEMORY_ERROR		| Message buffer couldn't be allocated											|
			| OTI_INVALID_PARAM		| Provided data is invalid														|

* See MapErrorToStatus() for extra error messages returned by ExchangeCmd()
*
* \ingroup UI
*********************************************************************************************************************/
OTI_STATUS StoreUIMode(UI_MODE mode, //!< [in] The mode to use and store. \see UI_MODE enum defintion.
	PMSG_BUFF msgBuf) //!< [in] Buffer for building message
{
    RESP_MSG_STRUCT resp;
    OTI_STATUS ret;
    BYTE cmd[] = {0xDF, 0x76, 0x01, 0x00};
    
    if(mode != MASTERCARD_UI && mode != EUROPE_UI)
        return OTI_INVALID_PARAM;

    WORD msgBufLen = msgBuf->Max_Length;

    /* Build 'User Interface Mode' message */
    cmd[3] = mode;
    ret = ExchangeCmd(OP_SET, cmd, 4, msgBuf->Data, &msgBufLen, &resp, SAT_DEFAULT_TIMEOUT);

    if(ret != OTI_OK)
    {
        LOG_DEBUG("SET UI MODE FAILED");
        return ret;
    }

    if(resp.succ == NULL)
    {
        if(resp.fail != NULL)
        {
            if(resp.failLen == 4)   // DF 76 01 ERR
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

    ret = LoadDefaultParams(msgBuf);
    if(ret != OTI_OK)
    {
        LOG_DEBUG("APPLY UI MODE FAILED");
        return ret;
    }

    return OTI_OK;
}// StoreUIMode()

// end
