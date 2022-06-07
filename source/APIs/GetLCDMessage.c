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
/************************** GetLCDMessage() ***********************************************************************//**
* Retrieves configured message number from the reader memory.
*
* This command is used to get the messages stored in the readers EEPROM. 
* It is especially useful for storing the content of a specific message before modifying it, and for restoring it later.

* @return	 OTI_STATUS				\n
			| Value					|	Description																	|
			|-----------------------|-------------------------------------------------------------------------------|
			| OTI_OK				| Function completed successfully				 								|
			| OTI_FAILED	    	| Command execution failed on Saturn reader			 							|
			| OTI_MEMORY_ERROR		| Message buffer couldn't be allocated											|
			| OTI_BUFFER_OVERFLOW	| Given buffer is not long enough												|
			| OTI_UNSUPPORTED		| Command not supported by Saturn reader										|
			| OTI_TLV_ERROR			| Error in building the protocol message encapsulating OTI command				|
			| OTI_UNKNOWN_DATA		| Retunred data does not match expected tag or lengh							|
			| OTI_DATA_LENGTH_OVERFLOW | TLV tag data length overflow (more than 900 Bytes)							|

* See MapErrorToStatus() for extra error messages returned by ExchangeCmd()
*
* \ingroup LCD
********************************************************************************************************************/
OTI_STATUS GetLCDMessage(	LCD_MESSAGE index,				 //!< [in] Message number		
							LCDMSG_SETTINGS_STRUCT *lcdResp, //!< [out] LCD Message structure for storing the retrieved message
							PMSG_BUFF cmdBuf, //!< [in] Buffer for building command message
							PMSG_BUFF msgBuf) //!< [in] Buffer for building response message
{
    RESP_MSG_STRUCT resp;
    OTI_STATUS ret;
    BYTE lcdCmd[] = {0xDF, 0x54, 0x02, 0x00, 0x00};
    BYTE *bufPtr;
    BYTE *endPtr;
    BYTE i;
    OTI_TLV tlvResp;

    WORD msgBufLen = msgBuf->Max_Length;

    /* We can figure out exact command length: three text lines to retrieve with fixed
     * formatting information (6 bytes each).
     */
    cmdBuf->Actual_Length = 15;    // (TAG + LEN + MSG_IDX + LINE_NB) * 3

    /* Build 'Get LCD' message */
    bufPtr = cmdBuf->Data;

    /* Line 1 */
    lcdCmd[3] = index;
    lcdCmd[4] = 0x01;
    memcpy(bufPtr, lcdCmd, sizeof(lcdCmd));
    bufPtr += sizeof(lcdCmd);

    /* Line 2 */
    lcdCmd[3] = index;
    lcdCmd[4] = 0x02;
    memcpy(bufPtr, lcdCmd, sizeof(lcdCmd));
    bufPtr += sizeof(lcdCmd);

    /* Line 3 */
    lcdCmd[3] = index;
    lcdCmd[4] = 0x03;
    memcpy(bufPtr, lcdCmd, sizeof(lcdCmd));
    bufPtr += sizeof(lcdCmd);

    if(bufPtr > cmdBuf->Data + cmdBuf->Actual_Length)
    {
        LOG_DEBUG("BUF OVERFLOW IN STORELCD");
        return OTI_BUFFER_OVERFLOW;
    }

    ret = ExchangeCmd(OP_GET, cmdBuf->Data, cmdBuf->Actual_Length, msgBuf->Data, &msgBufLen, &resp, SAT_DEFAULT_TIMEOUT);

    if(ret != OTI_OK)
    {
        LOG_DEBUG("GET LCDMSG CMD ERROR");
        return ret;
    }

    if(resp.fail != NULL)
    {
        if(resp.failLen == 4)   // DF 7D 01 ERR
            ret = MapErrorToStatus(resp.fail[3]);
        else
            ret = OTI_FAILED;

        return ret;
    }
    else if(resp.unSupp != NULL)
    {
        return OTI_UNSUPPORTED;
    }

    /* Parse the response tag (in order) and retrieve each line of text.
     * We're expecting exactly three response templates.
     */
    bufPtr = resp.succ;
    endPtr = resp.succ + resp.succLen;

    for(i = 0; i < 3; i++)
    {
        if(!BuildTlvFromRaw(bufPtr, (WORD)(endPtr - bufPtr), bufPtr, (WORD)(endPtr - bufPtr),&tlvResp))
        {
            return OTI_TLV_ERROR;
        }

        if(tlvResp.tag != 0xDF54 || tlvResp.len < 1)
        {
            return OTI_UNKNOWN_DATA;
        }

        if(tlvResp.len > LCD_MAXLGTH + 1)
        {
            return OTI_DATA_LENGTH_OVERFLOW;
        }

        /* TLV looks legal, parse its content */
        lcdResp->yCenter = ((tlvResp.value[0] & CENTER_Y) == CENTER_Y);
        lcdResp->format[i] = tlvResp.value[0] & ~CENTER_Y;    // Center Y is common to the three lines

        switch(i)
        {
            case 0:     // Line 1
                memcpy(lcdResp->L1, tlvResp.value + 1, tlvResp.len - 1);
                lcdResp->L1[tlvResp.len - 1] = 0x00;
                break;
            case 1:     // Line 2
                memcpy(lcdResp->L2, tlvResp.value + 1, tlvResp.len - 1);
                lcdResp->L2[tlvResp.len - 1] = 0x00;
                break;
            case 2:     // Line 3
                memcpy(lcdResp->L3, tlvResp.value + 1, tlvResp.len - 1);
                lcdResp->L3[tlvResp.len - 1] = 0x00;
                break;
        }
        bufPtr += tlvResp.size;
    }

    /* We got our three lines of text. Fill in the last items */
    lcdResp->index = index;
    lcdResp->setLines = LCD_LINE_ALL;

    /* Safety measure to avoid segfaults in case of incorrect (non NULL-terminated) string */
    lcdResp->L1[LCD_MAXLGTH] = 0x00;
    lcdResp->L2[LCD_MAXLGTH] = 0x00;
    lcdResp->L3[LCD_MAXLGTH] = 0x00;

	return ret;
}// GetLCDMessage()
// end
