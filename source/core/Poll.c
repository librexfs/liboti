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

/* Doxygen format:*/
/*****************************************************************************************************//**
*
FUNCTIONS:   Poll	- Polls for non EMV PICC.
********************************************************************************************************/

#include "Poll.h"

OTI_STATUS cmdPoll(PMSG_BUFF buf, BYTE opcode, POLL_MODE mode)
{
    BYTE cmdLen;
    BYTE *cmdPtr;

    if (!buf || !buf->Data || !buf->Max_Length)
        return OTI_MEMORY_ERROR;

    /* Figure out the length of command depending on opcode */
    if(opcode == OP_GET)
        cmdLen = 0x03;    // DF 7E 00
    else
        cmdLen = 0x04;    // DF 7E 01 <MODE>

        /* Fixed minimum size needed for command */
        if(buf->Max_Length < cmdLen)
            return OTI_BUFFER_OVERFLOW;
        cmdPtr = buf->Data;
        *cmdPtr = 0xDF;
        cmdPtr++;
        *cmdPtr  = 0x7E;
    
    if(opcode == OP_GET)
    {
        cmdPtr++;
        *cmdPtr = 0x00;
    }
    else
    {
        cmdPtr++;
        *cmdPtr = 0x01;
        cmdPtr++;
        *cmdPtr = (BYTE)mode;
    }

    /* Set final size of returned buffer */
    buf->Actual_Length = cmdLen;

    return OTI_OK;
} // cmdPoll()


OTI_STATUS Poll(BYTE opcode, POLL_MODE *mode, POLL_RESP_STRUCT *poll_resp, PMSG_BUFF msgBuf)

{
    RESP_MSG_STRUCT resp;
    OTI_STATUS ret = OTI_OK;
    BYTE  pollCmd[4];
    MSG_BUFF pollCmdPtr;
    pollCmdPtr.Data = pollCmd;
    pollCmdPtr.Max_Length = 0x04;

    /* Scope checking first */
    if((opcode == OP_GET) && !mode)
        return OTI_PARAM_MISSING;

    WORD msgBufLen = msgBuf->Max_Length;

    /* Build 'Poll' message */
    if(opcode == OP_GET)
        ret = cmdPoll(&pollCmdPtr, opcode, 0);
    else
        ret = cmdPoll(&pollCmdPtr, opcode, *mode);

    if(ret != OTI_OK)
    {
        return ret;
    }

    ret = ExchangeCmd(opcode, pollCmdPtr.Data, pollCmdPtr.Actual_Length, msgBuf->Data, &msgBufLen, &resp, 120000);

    if(ret != OTI_OK)
    {
        LOG_DEBUG("Poll CMD ERROR");
        return ret;
    }

    msgBuf->Actual_Length = msgBufLen;

    if(resp.succ == NULL)
    {
        if(resp.fail != NULL)
        {
            if(resp.failLen == 4)   // DF 7E 01 <ERR>
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

    /* Parse response if necessary (nothing to do for SET) */
    if(opcode == OP_GET)
    {
        if(resp.succLen == 4)   // DF 7E 01 <MODE>
            *mode = resp.succ[3];
        else
            ret = OTI_DATA_LENGTH_ERROR;
    }
    else if(opcode == OP_DO)
    {
        MSG_BUFF succ_buf = { resp.succ, resp.succLen, resp.succLen };
        ret = parsePollResp(&succ_buf, poll_resp);
    }

    return ret;

} // Poll()

OTI_STATUS parsePollResp(PMSG_BUFF msgBuf, POLL_RESP_STRUCT *poll_resp)
{
    static OTI_TLV curTag;
    BYTE *dataPtr;
    BYTE *endPtr;
    WORD respLen;
    WORD tagLen;

    memset(poll_resp, 0, sizeof(POLL_RESP_STRUCT));

    dataPtr = msgBuf->Data;
    endPtr = dataPtr + msgBuf->Actual_Length;

    /* Parse every TLV in the response tag FC. */
    if (*dataPtr != 0xFC)
    {
        LOG_DEBUG("WRONG TEMPLATE IN RESP");
        return OTI_INVALID_DATA;
    }
    dataPtr++;
    respLen = ParseLen(&dataPtr, endPtr, NULL);
    if (respLen == TLV_LEN_ERROR)
        return OTI_TLV_ERROR;

    /* Now dataPtr points on the list of EMV result TLVs */
    endPtr = dataPtr + respLen;
    while (dataPtr < endPtr)
    {
         if(!BuildTlvFromRaw(dataPtr, respLen, dataPtr, respLen,&curTag))
            return OTI_TLV_ERROR;

        tagLen = curTag.len;

        /* Assign object value to response structure member */
        switch (curTag.tag)
        {
        case 0xDF23:    // Track1
            if (tagLen > sizeof(poll_resp->track1_data))
            {
                return OTI_DATA_LENGTH_ERROR;
            }

            poll_resp->track1_len = tagLen;
            memcpy(poll_resp->track1_data, curTag.value, tagLen);
            poll_resp->properties |= RESP1_TRACK1;
            break;

        case 0xDF22:    // Track2
            if (tagLen > sizeof(poll_resp->track2_data))
            {
                return OTI_DATA_LENGTH_ERROR;
            }
            poll_resp->track2_len = tagLen;
            memcpy(poll_resp->track2_data, curTag.value, tagLen);
            poll_resp->properties |= RESP1_TRACK2;
            break;

        case 0xDF16:    // Card type
            if (tagLen > 1)
            {
                return OTI_DATA_LENGTH_ERROR;
            }
            poll_resp->type = *(curTag.value);
            poll_resp->properties |= POLL_PICC_TYPE;
            break;

        }

        dataPtr += curTag.size;
        respLen -= curTag.size;
    }

    return OTI_OK;
}
