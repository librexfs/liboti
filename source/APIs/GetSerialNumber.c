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
#include "frames.h"
#include "otilib.h"
#include "tlv.h"

/* Doxygen format:*/
/************************** GetSerialNumber() ******************************************************************//**
*  Gets the serial number from the reader.
*
* @return	 OTI_STATUS				\n
			| Value					|	Description																	|
			|-----------------------|-------------------------------------------------------------------------------|
			| OTI_OK				| Function completed successfully.				 								|
			| OTI_FAILED	    	| Command execution failed on Saturn reader			 							|
			| OTI_UNSUPPORTED		| Command not supported by Saturn reader										|
			| OTI_MEMORY_ERROR		| Message buffer couldn't be allocated											|
			| OTI_INVALID_DATA		| Provided data is invalid														|
			| OTI_BUFFER_OVERFLOW	| Given buffer is not long enough												|

* See MapErrorToStatus() for extra error messages returned by ExchangeCmd()
*
* \ingroup CFG
********************************************************************************************************/
OTI_STATUS GetSerialNumber(	PMSG_BUFF msgBuf) //!< [in/out] Buffer for building message and for return the device serial number
{
	RESP_MSG_STRUCT resp;
    OTI_STATUS ret;
    static OTI_TLV tlv;

    WORD msgBufLen = msgBuf->Max_Length;

    /* Build 'PCD Serial Number' message */
    ret = ExchangeCmd(OP_GET, (BYTE *)"\xDF\x4D\x00", 3, msgBuf->Data, &msgBufLen, &resp, SAT_DEFAULT_TIMEOUT);

    if(ret != OTI_OK)
    {
        LOG_DEBUG("SERIAL NUMBER CMD ERROR");
        return ret;
    }

    if(resp.succ == NULL)
    {
        if(resp.fail != NULL)
        {
            if(resp.failLen == 4)   // DF 4D 01 ERR
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

    if (!BuildTlvFromRaw(resp.succ, DWORDTOWORD(resp.succLen), resp.succ, DWORDTOWORD(resp.succLen), &tlv))
        return OTI_TLV_ERROR;

    if (msgBuf->Max_Length < tlv.len)
    {
        LOG_DEBUG("SERNUM BUF OVERFLOW");

        return OTI_BUFFER_OVERFLOW;
    }

    memcpy(msgBuf->Data, tlv.value, tlv.len);
    msgBuf ->Actual_Length = tlv.len;    // S/N shouldn't be more than 8 bytes

    return OTI_OK;
}// GetSerialNumber()

// end
