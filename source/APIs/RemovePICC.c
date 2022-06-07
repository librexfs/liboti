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
#include "RemovePICC.h"

OTI_STATUS cmdRemovePICC(BYTE **buf, WORD *bufLen)
{
	BYTE cmdLen;

	/* The RemovePICC command is exceedingly simple. This function
	 * is mostly provided for consistency.
	 */

	if(!buf || !bufLen)
		return OTI_MEMORY_ERROR;

	cmdLen = 0x03;	// DF 08 00

	/* Allocate command buffer if it's not provided */
	if(!*buf)
	{
		*buf = (BYTE *)malloc(cmdLen * sizeof(BYTE));
		if(!*buf)
			return OTI_MEMORY_ERROR;
	}
	else
	{
		/* Fixed minimum size needed for command */
		if(*bufLen < cmdLen)
			return OTI_BUFFER_OVERFLOW;
	}

	**buf		= 0xDF;
	*(*buf + 1)	= 0x08;
	*(*buf + 2)	= 0x00;

	/* Set final size of returned buffer */
	*bufLen = cmdLen;

	return OTI_OK;
} // cmdRemovePICC()

/* Doxygen format:*/
/************************** RemovePICC() **************************************************************************//**
* Performs PICC removal sequence.
*
* @return	 OTI_STATUS				\n
			| Value					|	Description																	|
			|-----------------------|-------------------------------------------------------------------------------|
			| OTI_OK				| Function completed successfully				 								|
			| OTI_MEMORY_ERROR		| Message buffer couldn't be allocated											|
			| OTI_FAILED	    	| Command execution failed on Saturn reader			 							|
			| OTI_UNSUPPORTED		| Command not supported by Saturn reader										|

* See MapErrorToStatus() for extra error messages returned by ExchangeCmd()
*
* \ingroup GENERAL
********************************************************************************************************************/
OTI_STATUS RemovePICC(void)
{
	RESP_MSG_STRUCT resp;
    OTI_STATUS ret = OTI_OK;
    BYTE *msgBuf = NULL;
    WORD  msgLen = PCD_MAX_BUFFER_SIZE;

	msgBuf = (BYTE *)malloc(msgLen * sizeof(BYTE));
	if(!msgBuf)
		return OTI_MEMORY_ERROR;

    /* Build 'Remove PICC' message. We could use the
	 * cmdRemovePICC utility function but that'd be overkill here.
	 */
    ret = ExchangeCmd(OP_DO, (BYTE *)"\xDF\x08\x00", 3, msgBuf, &msgLen, &resp, 20000);

    if(ret != OTI_OK)
    {
        LOG_DEBUG("Remove PICC CMD ERROR");
        free(msgBuf);
        return ret;
    }

    if(resp.succ == NULL)
    {
        if(resp.fail != NULL)
        {
            if(resp.failLen == 4)   // DF 08 01 <ERR>
                ret = MapErrorToStatus(resp.fail[3]);
            else
                ret = OTI_FAILED;

            free(msgBuf);
            return ret;
        }
        else
        {
            free(msgBuf);
            return OTI_UNSUPPORTED;
        }
    }

	/* No response expected, no further parsing needed */

	free(msgBuf);
	return OTI_OK;

} // RemovePICC()
