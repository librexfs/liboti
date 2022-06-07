/****************************************************************************

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

	FUNCTIONS:   TransparentTransportLayerControl	- Sets Transport Layer 
													  Control to Host or PCD
			 
	PROGRAMMER:  On Track Innovations Ltd.

	NOTES:       See [HOSTPCD 2.12 - 5.7.  Transparent - Transport Layer Control]

	REVISIONS:   2.0.00
*****************************************************************************
*/

#include <string.h>
#include <stdlib.h>
#include "otilib.h"
#include "protocol.h"
#include "TransparentTransportLayerControl.h"

OTI_STATUS cmdTransparentTransportLayerControl(BYTE **buf, WORD *bufLen, TLC_MODE mode)
{
	BYTE cmdLen;

	if(!buf || !bufLen)
		return OTI_MEMORY_ERROR;

	/* As of 2.12, only OP_DO is allowed */
	cmdLen = 0x04;	// DF 7A 01 <MODE>

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
	*(*buf + 1)	= 0x7A;
	*(*buf + 2)	= 0x01;
	*(*buf + 3)	= mode;


	/* Set final size of returned buffer */
	*bufLen = cmdLen;

	return OTI_OK;
} // cmdTransparentTransportLayerControl()


OTI_STATUS TransparentTransportLayerControl(TLC_MODE mode)
{
	RESP_MSG_STRUCT resp;
    OTI_STATUS ret = OTI_OK;
	BYTE  tlcCmd[4];
	BYTE *tlcCmdPtr = (BYTE *)tlcCmd;
	WORD  tlcCmdLen = 0x04;
    BYTE *msgBuf = NULL;
    WORD  msgLen = PCD_MAX_BUFFER_SIZE;

	msgBuf = (BYTE *)malloc(msgLen * sizeof(BYTE));
	if(!msgBuf)
		return OTI_MEMORY_ERROR;

    /* Build 'Transparent Transport Layer Control' message */
	ret = cmdTransparentTransportLayerControl(&tlcCmdPtr, &tlcCmdLen, mode);

	if(ret != OTI_OK)
	{
		free(msgBuf);
		return ret;
	}

    ret = ExchangeCmd(OP_DO, tlcCmd, tlcCmdLen, msgBuf, &msgLen, &resp, 0);

    if(ret != OTI_OK)
    {
        LOG_DEBUG("Transport Layer Control CMD ERROR");
        free(msgBuf);
        return ret;
    }

    if(resp.succ == NULL)
    {
        if(resp.fail != NULL)
        {
            if(resp.failLen == 4)   // DF 7A 01 <ERR>
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

} // TransparentTransportLayerControl()
