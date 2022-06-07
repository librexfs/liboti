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
/************************** RunApplication() **********************************************************************//**
*  Calls the Saturn's main FW code section and exits bootloader.
*
* This API instructs the PCD to go from the Boot Loader sector of the PCD into the Main FW sector of the PCD. \n
* This command should be executed after finishing the Flash programming sequence successfully.
* In case there is no Main FW or in case of verification Error, the PCD will stay in Boot Loader sector and response with a decline message.
* 
* @return	 OTI_STATUS				\n
			| Value					|	Description																	|
			|-----------------------|-------------------------------------------------------------------------------|
			| OTI_OK				| Function completed successfully.				 								|
			| OTI_FAILED	    	| Command execution failed on Saturn reader			 							|
			| OTI_MEMORY_ERROR		| Message buffer couldn't be allocated											|
			| OTI_INVALID_PARAM		| Provided data is invalid														|
			| OTI_DATA_LENGTH_ERROR | Error in command length encoding.												|

* See MapErrorToStatus() for extra error messages returned by ExchangeCmd()
*
* \ingroup GENERAL
*********************************************************************************************************************/
OTI_STATUS RunApplication(PMSG_BUFF msgBuf) //!< [in] Buffer for building message

{
 	  RESP_MSG_STRUCT resp;
    OTI_STATUS 		ret;

    WORD msgBufLen = msgBuf->Max_Length;

    /* We may be rebooting so use a big enough timeout */
    ret = ExchangeCmd(OP_DO, (BYTE *)"\xDF\x6E\x00", 3, msgBuf->Data, &msgBufLen, &resp, 20000);

    if(ret != OTI_OK)
    {
        LOG_DEBUG("Run Application CMD ERROR");
        return ret;
    }

    msgBuf->Actual_Length = msgBufLen;

    if(resp.succ == NULL)
    {
        if(resp.fail != NULL)
        {
            if(resp.failLen == 4)   // DF 6E 01 ERR
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
} //RunApplication()

// end
