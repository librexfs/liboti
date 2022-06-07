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
/************************** DeleteCAPublicKey() *******************************************************************//**
* Deletes the CA Public Key in the Saturn (SAM Module).
*
* The Public Key will be uniquely identified by the <rid, index> pair. \n
* This function will return an error if there is no SAM card inside the Saturn reader, 
* or if the SAM isn't properly initialized ('pki storage error' displayed at boot time).
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
* \ingroup KEY
********************************************************************************************************************/


OTI_STATUS DeleteCAPublicKey(	BYTE* rid, //!< [in] RID (5 bytes) of the key to delete
								BYTE index, //!< [in] index of the key for the specified RID
								PMSG_BUFF msgBuf) //!< [in] Buffer for building message
{
    RESP_MSG_STRUCT resp;
    OTI_STATUS ret;
    BYTE cmdBuf[] = {0xDF, 0x51, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01};

    /* Build 'Public Keys' message */
    memcpy(cmdBuf + 3, rid, 5);
    cmdBuf[8] = index;
    WORD msgBufLen = msgBuf->Max_Length;
    ret = ExchangeCmd(OP_SET, cmdBuf, 10, msgBuf->Data, &msgBufLen, &resp, SAT_DEFAULT_TIMEOUT);
    
    if(ret != OTI_OK)
    {
        LOG_DEBUG("DELKEY CMD ERROR");
        return ret;
    }
    msgBuf->Actual_Length = msgBufLen;
    if(resp.succ == NULL)
    {
        if(resp.fail != NULL)
        {
            if(resp.failLen == 4)   // DF 51 01 ERR
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
}// DeleteCAPublicKey()


/* Doxygen format:*/
/************************** DeleteAllCAKeys() *********************************************************************//**
* Deletes all the CA Public Keys in the Saturn (SAM Module).
*
* The Public Keys list will be retreived with a first command sent to the reader, 
* then a second command will delete them all at once. \n
* This function will return an error if there is no SAM card inside the Saturn reader, 
* or if the SAM isn't properly initialized ('pki storage error' displayed at boot time).
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
* \ingroup KEY
********************************************************************************************************************/


OTI_STATUS DeleteAllCAKeys(PMSG_BUFF msgBuf) //!< [in] Buffer for building message
{
    RESP_MSG_STRUCT resp;
    OTI_STATUS ret;
    BYTE *bufPtr;
    BYTE *cmdPtr;
    BYTE *curr_rid;
    OTI_TLV respTlv;
    BYTE nbKeys = 0;
    int i;

    WORD cmdBufLen = msgBuf->Max_Length;

    ret = ExchangeCmd(OP_GET, (BYTE *)"\xDF\x5C\x00", 3, msgBuf->Data, &cmdBufLen, &resp, SAT_DEFAULT_TIMEOUT);

    if(ret != OTI_OK)
    {
        LOG_DEBUG("PKLIST CMD ERROR");
        return ret;
    }

    if(resp.succ == NULL)
    {
        if(resp.fail != NULL)
        {
            if(resp.failLen == 4)   // DF 51 01 ERR
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

    if (!BuildTlvFromRaw(resp.succ, DWORDTOWORD(resp.succLen), resp.succ, DWORDTOWORD(resp.succLen), &respTlv))
        return OTI_TLV_ERROR;

    if(respTlv.len == 0)
    {
        return OTI_OK;
    }

    bufPtr = respTlv.value;
    cmdPtr = msgBuf->Data;
    while(bufPtr < respTlv.value+respTlv.len)
    {
        curr_rid = bufPtr;
        bufPtr += 6;

        for(i = 0 ; i < curr_rid[5] ; i++)
        {
            memcpy(cmdPtr, "\xDF\x51\x07", 3);
            cmdPtr += 3;
            memcpy(cmdPtr, curr_rid, 5);
            cmdPtr += 5;
            *cmdPtr = *bufPtr;
            bufPtr += 1;
            cmdPtr += 1;
            *cmdPtr = 0x01;
            cmdPtr += 1;
        }
        nbKeys += i;
    }

    cmdBufLen = PCD_MAX_BUFFER_SIZE;
    ret = ExchangeCmd(OP_SET, msgBuf->Data, (WORD)(cmdPtr - msgBuf->Data), msgBuf->Data, &cmdBufLen, &resp, nbKeys*FIRST_BYTE_TIMEOUT);

    if(ret != OTI_OK)
    {
        LOG_DEBUG("DELPK CMD ERROR");
        PrintBuf(msgBuf->Data, (WORD)(cmdPtr - msgBuf->Data));
        return ret;
    }

    if(resp.succ == NULL)
    {
        if(resp.fail != NULL)
        {
            if(resp.failLen == 4)   // DF 51 01 ERR
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
}// DeleteAllCAKeys()

// end
