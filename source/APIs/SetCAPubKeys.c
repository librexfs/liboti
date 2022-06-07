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
/************************** SetCAPublicKey() **********************************************************************//**
* Sets the CA Public Key in the Saturn reader (SAM Module). 
*
* The Public Key will be uniquely identified by the <rid, index> pair. \n
* This function will return an error if there is no SAM card inside the Saturn reader, 
* or if the SAM isn't properly initialized ('pki storage error' displayed at boot time).
* 
* @return	 OTI_STATUS				\n
			| Value					|	Description																	|
			|-----------------------|-------------------------------------------------------------------------------|
			| OTI_OK				| Function completed successfully				 								|
			| OTI_MEMORY_ERROR		| Message buffer couldn't be allocated											|
			| OTI_FAILED	    	| Command execution failed on Saturn reader			 							|
			| OTI_UNSUPPORTED		| Command not supported by Saturn reader										|
			| OTI_INVALID_PARAM		| Provided parameter is invalid													|

* See MapErrorToStatus() for extra error messages returned by ExchangeCmd()
*
* \ingroup KEY
********************************************************************************************************************/
OTI_STATUS SetCAPublicKey(OTI_CAPK_STRUCT* capk, /**<	[in] CA public Key.
														See OTI_CAPK_STRUCT definition */
														PMSG_BUFF cmdBuf, //!< [in] Buffer for building command data
														PMSG_BUFF msgBuf) //!< [in] Buffer for building message
{
    RESP_MSG_STRUCT resp;
    OTI_STATUS ret;
    BYTE *bufPtr;

    /* Build 'Public Keys' message */
    bufPtr = cmdBuf->Data;
    memcpy(bufPtr, "\xDF\x51", 2);
    bufPtr += 2;
    WORD cmdBufLen = 31 + capk->exp_len + capk->mod_len;    // Public Keys data len
    if (cmdBufLen > cmdBuf->Max_Length)
        return OTI_BUFFER_OVERFLOW;

    PackLen(cmdBufLen, &bufPtr, bufPtr + cmdBufLen, NULL); // Increments bufPtr
    memcpy(bufPtr, capk->rid, 5);
    bufPtr += 5;
    *bufPtr = capk->index;
    bufPtr++;
    *bufPtr = 0x00;
    bufPtr++;
    *bufPtr = capk->hash_algo_indic;
    bufPtr++;
    *bufPtr = capk->key_algo_indic;
    bufPtr++;
    *bufPtr = capk->mod_len;
    bufPtr++;
    memcpy(bufPtr, capk->mod, capk->mod_len);
    bufPtr += capk->mod_len;
    *bufPtr = capk->exp_len;
    bufPtr++;
    memcpy(bufPtr, capk->exp, capk->exp_len);
    bufPtr += capk->exp_len;
    memcpy(bufPtr, capk->hash_result, 20);
    bufPtr += 20;
    cmdBufLen = bufPtr - cmdBuf->Data;       // Public keys full cmd len
    WORD msgBufLen = msgBuf->Max_Length;
    ret = ExchangeCmd(OP_SET, cmdBuf->Data, cmdBufLen, msgBuf->Data, &msgBufLen, &resp, SAT_DEFAULT_TIMEOUT);

    if (ret != OTI_OK)
    {
        LOG_DEBUG("SETKEY CMD ERROR");
        return ret;
    }
    msgBuf->Actual_Length = msgBufLen;
    if (resp.succ == NULL)
    {
        if (resp.fail != NULL)
        {
            if (resp.failLen == 4)   // DF 51 01 ERR
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
}// SetCAPublicKey()

// end




