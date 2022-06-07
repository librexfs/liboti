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
/************************** GetFirmwareVersion() ******************************************************************//**
* Gets the firmware version from the reader.
*
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
OTI_STATUS GetFirmwareVersion(BYTE fModule,//!< [in]  The module index (see 2.5.27 Versions section in OTI_Host-PCD_CommunicationProtocol document
								PMSG_BUFF msgBuf) //!< [in] Buffer for building message
{
    RESP_MSG_STRUCT resp;
    OTI_STATUS ret;
    OTI_TLV succ_tlv;
    BYTE cmd[] = { 0xDF, 0x4E, 0x01, 0x01 };

    cmd[3] = fModule;
    WORD msgBufLen = msgBuf->Max_Length;

    /* Build 'Stop Macro' message */
    ret = ExchangeCmd(OP_GET, cmd, 4, msgBuf->Data, &msgBufLen, &resp, SAT_DEFAULT_TIMEOUT);

    if(ret != OTI_OK)
    {
        LOG_DEBUG("GETFWVER CMD ERROR");
        return ret;
    }

    if(resp.succ == NULL)
    {
        if(resp.fail != NULL)
        {
            if(resp.failLen == 4)   // DF 7D 01 ERR
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

    if (!BuildTlvFromRaw(resp.succ, DWORDTOWORD(resp.succLen), resp.succ, DWORDTOWORD(resp.succLen), &succ_tlv))
        return OTI_TLV_ERROR;

    if(succ_tlv.tag != 0xDF4E)
    {
        return OTI_INVALID_DATA;
    }
    if (succ_tlv.len > msgBuf->Max_Length)
    {
        return OTI_BUFFER_OVERFLOW;
    }

    memmove(msgBuf->Data, succ_tlv.value, succ_tlv.len);
    msgBuf->Actual_Length = (BYTE)succ_tlv.len;        // F/W version shouldn't be longer than 255 chars

    return OTI_OK;
}// GetFirmwareVersion()

// end
