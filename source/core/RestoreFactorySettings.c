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
#include "RestoreFactorySettings.h"

OTI_STATUS cmdRestoreFactorySettings(BYTE **buf, WORD *bufLen)
{
    BYTE cmdLen;

    /* The RestoreFactorySettings command is exceedingly simple. 
     * This function is mostly provided for consistency.
     */

    if(!buf || !bufLen)
        return OTI_MEMORY_ERROR;

    cmdLen = 0x03;    // DF 4C 00

    /* Fixed minimum size needed for command */
    if(*bufLen < cmdLen)
        return OTI_BUFFER_OVERFLOW;

    **buf       = 0xDF;
    *(*buf + 1) = 0x4C;
    *(*buf + 2) = 0x00;

    /* Set final size of returned buffer */
    *bufLen = cmdLen;

    return OTI_OK;
} // cmdRemovePICC()

/************************** RestoreFactorySettings() ***************************************************************//**
* Reverts all settings to factory except baudrate and LCD messages.
*
* @return	 OTI_STATUS				\n
			| Value					|	Description																	|
			|-----------------------|-------------------------------------------------------------------------------|
			| OTI_OK				| Function completed successfully				 								|
			| OTI_FAILED	    	| Command execution failed on Saturn reader			 							|
			| OTI_MEMORY_ERROR		| Message buffer couldn't be allocated											|
			| OTI_BUFFER_OVERFLOW	| Given buffer is not long enough												|
			| OTI_UNSUPPORTED		| Command not supported by Saturn reader										|

* See MapErrorToStatus() for extra error messages returned by ExchangeCmd()
*
* \ingroup INTERNAL
********************************************************************************************************************/
OTI_STATUS RestoreFactorySettings(BYTE *msgBuf, //!< [in] Buffer for response message
	WORD *msgBufLen) //!< [in] Buffer length for response message. If msgBufLen = 0 then method return the length or the buffer message and return with OTI_STATUS = OTI_BUFFER_LENGTH and without execute the message. 

{
    RESP_MSG_STRUCT resp;
    OTI_STATUS ret = OTI_OK;

    if (*msgBufLen == 0)
    {
        *msgBufLen = PCD_MAX_BUFFER_SIZE;
        return OTI_BUFFER_LENGTH;
    }

    /* Build 'Restore Factory Settings' message. We could use the
     * cmdRestoreFactorySettings utility function but that'd be overkill here.
     */
    ret = ExchangeCmd(OP_DO, (BYTE *)"\xDF\x4C\x00", 3, msgBuf, msgBufLen, &resp, 20000);

    if(ret != OTI_OK)
    {
        LOG_DEBUG("Restore Factory Settings CMD ERROR");
        return ret;
    }

    if(resp.succ == NULL)
    {
        if(resp.fail != NULL)
        {
            if(resp.failLen == 4)   // DF 4C 01 <ERR>
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

    /* No response expected, no further parsing needed */

    return OTI_OK;

} // RestoreFactorySettings()

// end
