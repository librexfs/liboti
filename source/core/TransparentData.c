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
#include "tlv.h"
#include "oti_types.h"

/************************** TransparentData() *********************************************************************//**
* Sends transparent data to the reader.
*
* OTI protocol gives the Host the ability to send/receive messages directly to/from the PICC in a transparent manner.
* See more details about transparent mode in the "oti Host-PCD Protocol" docuemnt "Transparent Data" Tag (DF 11)
*
* @return	 OTI_STATUS				\n
			| Value					|	Description																	|
			|-----------------------|-------------------------------------------------------------------------------|
			| OTI_OK				| Function completed successfully				 								|
			| OTI_MEMORY_ERROR		| Message buffer couldn't be allocated											|
			| OTI_BUFFER_OVERFLOW	| Given buffer is not long enough												|
			| OTI_UNSUPPORTED		| Command not supported by Saturn reader										|
			| OTI_FAILED	    	| Command execution failed on Saturn reader			 							|
			| OTI_INVALID_DATA		| Provided data is invalid														|

* See MapErrorToStatus() for extra error messages returned by ExchangeCmd()
*
* \ingroup INTERNAL
********************************************************************************************************************/
OTI_STATUS TransparentData(	PMSG_BUFF data,			//!< [in] Data to send
							PMSG_BUFF respBuf,		//!< [out] Response data 
							PMSG_BUFF msgBuf) //!< [in] Buffer for response message
{
    RESP_MSG_STRUCT resp;
    OTI_STATUS ret;
    OTI_TLV cmdTlv;
    OTI_TLV respTlv;

    WORD msgBufLen = msgBuf->Max_Length;

    /* Build 'Transparent Data' message */
    if (!BuildTlv(0xDF11, data->Actual_Length, data->Data, msgBuf->Data+5, msgBuf->Max_Length-5, &cmdTlv)) // we keep 5 bytes because we build the data TLV in the same buffer where the frame will be built
    {
        return OTI_TLV_ERROR;
    }

    ret = ExchangeCmd(OP_DO, cmdTlv.rawTlv, cmdTlv.size, msgBuf->Data, &msgBufLen, &resp, 1000);

    if(ret != OTI_OK)
    {
        LOG_DEBUG("TransparentData CMD ERROR");
        return ret;
    }

    msgBuf->Actual_Length = msgBufLen;

    if(resp.succ == NULL)
    {
        if(resp.fail != NULL)
        {
            if(resp.failLen == 4)   // DF 11 01 ERR
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

    /* Store transparent response in output buffer.*/
    if (!BuildTlvFromRaw(resp.succ, DWORDTOWORD(resp.succLen), msgBuf->Data, msgBuf->Max_Length, &respTlv))
        return OTI_TLV_ERROR;

    /* Check that this is the response we want */
    if(respTlv.tag != 0xDF11)
    {
        return OTI_INVALID_DATA;
    }

    if((respBuf->Max_Length < respTlv.len))
    {
        /* Not enough room for response */
        return OTI_BUFFER_OVERFLOW;
    }

    memmove(respBuf->Data, respTlv.value, respTlv.len);
    respBuf->Actual_Length = respTlv.len;

    return OTI_OK;

} // TransparentData()

