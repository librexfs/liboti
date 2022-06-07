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
* Implementation of the HostPCD Protocol - transport level.
*
********************************************************************************************************/

#include "protocol.h"
#include "platform_utils.h"
#include "serial.h"


void PrintBuf(BYTE *buf, WORD bufLen)
{
    static BYTE tmpBuf[49];
    WORD i;

    // Write buffer bytes in chunks of 16 bytes
    for (i = 0; i < bufLen; i++)
    {
        sprintf((char *)&tmpBuf[(i % 16) * 3], "%02X ", buf[i]);

        // Flush our buffer before overwriting from index 0 again
        if ((i % 16) == 15)
        {
            tmpBuf[48] = 0x00;
            LOG_OUTPUT("%s\r\n", (char *)tmpBuf);
        }
    }

    // We're out, have we written the last chunks?
    if (((i - 1) % 16) < 15)
    {
        tmpBuf[(i % 16) * 3] = 0x00;
        LOG_OUTPUT("%s", (char *)tmpBuf);
    }

    LOG_OUTPUT("\r\n");
}

BYTE ComputeLrc(BYTE *buf, DWORD len)
{
    BYTE lrc = 0x00;    //Initial value
    while (len--)
    {
        lrc = lrc ^ (*buf++);
    }
    return lrc;
} // ComputeLrc

OTI_STATUS CheckFrame(BYTE *buf, DWORD len)
{
    BYTE lrc;

    if (len < 6)
        return OTI_DATA_LENGTH_ERROR;

    if (buf[0] != MSG_STX)
        return OTI_STX_MISSING;

    if (buf[len - 1] != MSG_ETX)
        return OTI_ETX_MISSING;

    lrc = buf[len - 2];                      // Get LRC from msg
    if (lrc == ComputeLrc(buf, len - 2))    // Galculate LRC and verify.
        return OTI_OK;
    else
        return OTI_LRC_ERROR;
} // CheckFrame

OTI_STATUS SendRawCmd(BYTE opcode, BYTE *cmdData, WORD cmdLen, BYTE *msgBuf, WORD *msgBufLen)
{
    OTI_STATUS ret;

    ret = BuildMsg(opcode, cmdData, cmdLen, msgBuf, msgBufLen);
    if (ret != OTI_OK)
    {
        LOG_DEBUG("BUILD MSG FAILED\r\n");
        return ret;
    }

    ret = Send(msgBuf, *msgBufLen);

    if (ret != OTI_OK)
    {
        LOG_DEBUG("SEND RAW MSG FAILED\r\n");
    }

    return ret;
}

OTI_STATUS ExchangeCmd_UnSync(BYTE opcode, BYTE *cmdData, WORD cmdLen, BYTE *msgBuf, WORD *msgBufLen, RESP_MSG_STRUCT *resp, DWORD respTimeout)
{
    OTI_STATUS ret;
    WORD maxMsgBufLen;
    WORD respLen;

    maxMsgBufLen = *msgBufLen;   // Save message buffer size
    respLen = maxMsgBufLen; // Default size for response buffer
    ret = BuildMsg(opcode, cmdData, cmdLen, msgBuf, msgBufLen);
    if (ret != OTI_OK)
    {
        LOG_DEBUG("BUILD MSG FAILED\r\n");
        return ret;
    }

    ret = SendMsg(msgBuf, *msgBufLen, msgBuf, &respLen);

    if (ret != OTI_OK)
    {
        LOG_DEBUG("SEND MSG FAILED\r\n");

        /* Adjust size of recieved data (ACK or NAK or other) */
        *msgBufLen = respLen;
        return ret;
    }

    /* Receive response (with timeout if specified) and check it's complete */
    *msgBufLen = maxMsgBufLen;    // Restore original full buffer size
    ret = RecvMsg(msgBuf, msgBufLen, resp, respTimeout);

    if (ret != OTI_OK)
    {
        LOG_DEBUG("RECEIVE RESP FAILED\r\n");
        return ret;
    }

    return OTI_OK;

} // ExchangeCmd_Unsync()


OTI_STATUS ExchangeCmd(BYTE opcode, BYTE *cmdData, WORD cmdLen, BYTE *msgBuf, WORD *msgBufLen, RESP_MSG_STRUCT *resp, DWORD respTimeout)
{
    OTI_STATUS status;
    if (!SAT_Mutex_Acquire(SAT_DEFAULT_TIMEOUT + 200))
        return OTI_READER_BUSY;
    status = ExchangeCmd_UnSync(opcode, cmdData, cmdLen, msgBuf, msgBufLen, resp, respTimeout);
    SAT_Mutex_Release();
    return status;
}

OTI_STATUS BuildMsg(BYTE opcode, BYTE *cmdData, WORD cmdLen, BYTE *buf, WORD *bufLen)
{
    WORD msgLen = 5;    // Minimal length: STX + UNIT + OPCODE + LRC + ETX (1 byte each)
    BYTE *bufPtr;

    /* LENGTH field applies to the whole message, including LENGTH itself. */
    msgLen += cmdLen;    // Message length excluding LENGTH itself
    if (msgLen < 0x7F) {                // 0x7E or less
        /* LENGTH fits in one byte (total message length <= 7F) */
        msgLen += 1;
    }
    else if (msgLen < 0xFE) {        // 0xFD or less
        /* LENGTH fits in two bytes (total message length <= FF) */
        msgLen += 2;
    }
    else {    // 0xFFFC or less
        /* LENGTH fits in three bytes (total message length <= FFFF) */
        msgLen += 3;
    }

    /* Paranoid check */
    if (msgLen > *bufLen)
        return OTI_BUFFER_OVERFLOW;

    /* Start building message (STX | LEN | UNIT | OPCODE | DATA | LRC | ETX) */
    bufPtr = buf;

    *bufPtr = MSG_STX;
    bufPtr++;

    if (!PackLen(msgLen, &bufPtr, buf + msgLen, NULL))
        return OTI_TLV_ERROR;

    *bufPtr = 0x00;    // Unit number
    bufPtr++;

    *bufPtr = opcode;
    bufPtr++;

    memmove(bufPtr, cmdData, cmdLen);
    bufPtr += cmdLen;

    *bufPtr = ComputeLrc(buf, bufPtr - buf);
    bufPtr++;

    *bufPtr = MSG_ETX;
    bufPtr++;

    *bufLen = msgLen;

    return OTI_OK;
} // BuildFrame

OTI_STATUS SendMsg(BYTE *msg, WORD msgLen, BYTE *respBuf, WORD *respBufLen)
{
    WORD readLen;
    OTI_STATUS ret;

    /* There is a minimum size for response buffer */
    if (*respBufLen < ACK_MSG_LEN)
    {
        LOG_DEBUG("BUFFER TOO SMALL\r\n");
        return OTI_BUFFER_OVERFLOW;
    }

    /* 1. Send command MSG */
    if (GetConfig())
    {
        if (gConf.debug)
        {
            LOG_TRACE(">>>\r\n");
            PrintBuf(msg, msgLen);
        }
    }
    Sat_Clear_Incoming();
    if (Send(msg, msgLen) != OTI_OK)
    {
        LOG_DEBUG("SEND MSG ERROR\r\n");
        return OTI_COM_ERROR;
    }

    /* 2. Get and verify ACK */
    readLen = ACK_MSG_LEN;    // May be 1 byte shorter if we get a NAK
    ret = GetFullFrame(respBuf, &readLen, 3000);

    if (ret != OTI_OK)
    {
        LOG_DEBUG("GET ACK ERROR\r\n");
        return OTI_COM_ERROR;
    }

    /* 3. Check message validity through LRC */
    ret = CheckFrame(respBuf, readLen);
    if (ret != OTI_OK)
    {
        LOG_DEBUG("N/ACK FRAME ERROR\r\n");
        return ret;
    }

    if (respBuf[3] == OP_NAK)
    {
        LOG_DEBUG("NAK");
        return MapNAKToStatus(respBuf[4]);
    }

    return OTI_OK;
}

OTI_STATUS RecvMsg(BYTE *respBuf, WORD *respBufLen, RESP_MSG_STRUCT *respData, DWORD respTimeout)
{
    BYTE *bufPtr;
    BYTE *endPtr;
    OTI_STATUS ret;

    ret = GetFullFrame(respBuf, respBufLen, respTimeout);
    if (ret != OTI_OK)
    {
        LOG_DEBUG("GET RESP ERROR\r\n");
        return ret;
    }

    ret = CheckResp(respBuf, respBufLen, &bufPtr, &endPtr);
    if (ret != OTI_OK)
    {
        LOG_DEBUG("RESP FRAME ERROR (respBufLen: %d)\r\n", *respBufLen);
        PrintBuf(respBuf, *respBufLen);
        return ret;
    }

    /* 5. Parse response message */
    ret = ParseResp(bufPtr, endPtr, respData);
    if (ret != OTI_OK)
    {
        LOG_DEBUG("RESP PARSE ERROR\r\n");
        return ret;
    }

    if ((respData->succ == NULL) &&
        (respData->fail == NULL) &&
        (respData->unSupp == NULL))
    {
        LOG_DEBUG("RESP TEMPLATE MISSING\r\n");
        return OTI_DATA_MISSING;
    }

    return OTI_OK;
}

OTI_STATUS GetFullFrame(BYTE *respBuf, WORD *respBufLen, DWORD respTimeout)
{
    /* Read OTI frame incrementally to only get the exact number
     * of bytes.
     */
    OTI_STATUS ret;
    WORD headLen;
    WORD frameSize;
    BYTE *bufPtr;

    /* Read STX + max_size(LEN) -> 3 */
    headLen = 4;

    ret = Receive(respBuf, &headLen, respTimeout);

    /* We must have received all our 4 bytes, an OTI frame can't
     * be shorter than 6 anyway.
     */
    if ((ret != OTI_OK) || (headLen != 4))
    {
        LOG_DEBUG("GET RESP ERROR\r\n");
        return OTI_COM_ERROR;
    }

    /* Parse length bytes and check that return buffer is big enough */
    bufPtr = &respBuf[1];
    frameSize = ParseLen(&bufPtr, &respBuf[4], NULL);

    if (*respBufLen < frameSize)
    {
        LOG_DEBUG("BUFFER TOO SMALL FOR FRAME\r\n");
        return OTI_DATA_LENGTH_OVERFLOW;
    }

    /* We have the total number of bytes to read, and we got 4 already */
    *respBufLen = frameSize - 4;

    ret = Receive(&respBuf[4], respBufLen, respTimeout);

    if ((ret != OTI_OK) || (*respBufLen != (frameSize - 4)))
    {
        LOG_DEBUG("GET RESP ERROR\r\n");
        return OTI_COM_ERROR;
    }

    /* Set response length to exact, full size */
    *respBufLen = frameSize;

    if (GetConfig())
    {
        if (gConf.debug)
        {
            LOG_TRACE("<<<\r\n");
            PrintBuf(respBuf, *respBufLen);
        }
    }

    return OTI_OK;
}

OTI_STATUS CheckResp(BYTE *respBuf, WORD *respBufLen, BYTE **bufPtr, BYTE **endPtr)
{
    OTI_STATUS ret;
    WORD expectSize;

    /* We WILL get garbage after a SetSpeed/Cancel command so
     * we may miss ETX. If we don't get size error, there will
     * be enough data to do a size check at least. */
    ret = CheckFrame(respBuf, *respBufLen);
    if (ret != OTI_OK && ret != OTI_ETX_MISSING)
    {
        LOG_DEBUG("CHECK RESP FRAME ERROR\r\n");
        return OTI_LRC_ERROR;
    }

    *bufPtr = (respBuf + 1);     // Point on first LENGTH byte
    *endPtr = (respBuf + *respBufLen);
    expectSize = ParseLen(bufPtr, *endPtr, NULL);

    if (expectSize == TLV_LEN_ERROR)
    {
        LOG_DEBUG("RESP SIZE INVALID\r\n");
        return OTI_INVALID_DATA;
    }

    if (expectSize > *respBufLen)
    {
        LOG_DEBUG("RESP INCOMPLETE\r\n");
        return OTI_INCOMPLETE_DATA;
    }

    if (expectSize < *respBufLen)
    {
        /* Got trailing garbage, try stripping it and re-check */
        LOG_DEBUG("RESP TOO LONG\r\n");
        *respBufLen = expectSize;
        *endPtr = (respBuf + *respBufLen);
    }

    /* Re-check after possible stripping */
    ret = CheckFrame(respBuf, *respBufLen);
    if (ret != OTI_OK)
    {
        LOG_DEBUG("RESP FRAME ERROR\r\n");
        return ret;
    }

    /* Put bufPtr at the very beginning of DATA field in response (skip UNIT and OPCODE)
     * and endPtr at the end of the value field (pointing on LRC) */
    *bufPtr += 2;
    *endPtr -= 2;

    return OTI_OK;
}

OTI_STATUS ParseResp(BYTE *bufPtr, BYTE *endPtr, RESP_MSG_STRUCT *respData)
{
    OTI_TLV respTlv;
    OTI_STATUS ret = OTI_OK;

    respData->succ = NULL;
    respData->succLen = 0;
    respData->fail = NULL;
    respData->failLen = 0;
    respData->unSupp = NULL;
    respData->unSuppLen = 0;

    /* According to OTI Host-Pcd Communication Protocol, ch 3.2.4, Table 8, we're
     * expecting at least one of three TLV templates (Success, Unsupported, Failed).
     */
    while ((bufPtr < endPtr) && (ret == OTI_OK))
    {
         if(!BuildTlvFromRaw(bufPtr, (WORD)(endPtr - bufPtr), bufPtr, (WORD)(endPtr - bufPtr),&respTlv))
        {
            LOG_DEBUG("TLV PARSE ERROR\r\n");
            return OTI_TLV_ERROR;
        }

        switch (respTlv.tag)
        {
        case 0xFF01:
            /* Successful command(s), TLV contains return value(s) */
            respData->succ = respTlv.value;
            respData->succLen = respTlv.len;
            break;

        case 0xFF02:
            /* Unsupported command(s), TLV contains command tag(s) */
            respData->unSupp = respTlv.value;
            respData->unSuppLen = respTlv.len;
            break;

        case 0xFF03:
            /* Failed command(s), TLV contains error code(s) */
            respData->fail = respTlv.value;
            respData->failLen = respTlv.len;
            break;

        default:
            /* Unknown TLV. This is a protocol break or provided
             * data is invalid.
             */
            LOG_DEBUG("UNKNOWN RESP TLV\r\n");
            ret = OTI_INVALID_DATA;
        }

        /* Move forward to potential next TLV in response */
        bufPtr += respTlv.size;
        //DeleteTlv(respTlv);
    }

    return ret;
}

OTI_STATUS ParseData(BYTE *data, WORD dataLen, OTI_STATUS(*callback)(FRAME_TYPE, BYTE, WORD, BYTE *, WORD))
{
    OTI_STATUS ret;
    FRAME_TYPE type = MSG_TYPE_CMD;
    BYTE opcode;
    BYTE *bufPtr;
    BYTE *endPtr;
    OTI_TLV tlv;

    /* Use existing ParseResp API to check the frame and jump to
     * the first TLV object in it.
     */
    ret = CheckResp(data, &dataLen, &bufPtr, &endPtr);
    if (ret != OTI_OK)
        return ret;

    /* From there we can already figure out opcode and whether we
     * have a NAK frame. Callback and stop here.
     */
    opcode = *(bufPtr - 1);

    if ((dataLen == 7) && (opcode == OP_NAK))
    {
        type = MSG_TYPE_NAK;
        return callback(type, opcode, 0, bufPtr, 1);
    }

    /* Otherwise, parse the actual data. ACK is a special case of a TLV
     * with TAG and LEN to zero.
     */
    while (bufPtr < endPtr)
    {
        dataLen = endPtr - bufPtr;
         if(!BuildTlvFromRaw(bufPtr, dataLen, bufPtr, dataLen,&tlv))
            return OTI_TLV_ERROR;

        /* Check whether this is a response or ACK template */
        switch (tlv.tag)
        {
        case 0x00:        type = MSG_TYPE_ACK;
            break;

        case 0xFF01:    type = MSG_TYPE_SUCCESS;
            break;

        case 0xFF02:    type = MSG_TYPE_UNSUPPORTED;
            break;

        case 0xFF03:    type = MSG_TYPE_FAILURE;
            break;

        default:
            break;
        }

        /* For responses, skip directly to nested TLVs */
        if ((tlv.tag == 0xFF01) || (tlv.tag == 0xFF02) || (tlv.tag == 0xFF03))
        {
            bufPtr = tlv.value;
            //DeleteTlv(tlv);
            continue;
        }

        /* Otherwise, use callback with all the gathered data */
        ret = callback(type, opcode, (WORD)tlv.tag, tlv.value, tlv.len);

        if (ret != OTI_OK)
        {
            return ret;
        }

        /* Done, move to the next TLV if applicable */
        bufPtr += tlv.size;
    }

    return OTI_OK;
}

OTI_STATUS MapNAKToStatus(BYTE errNak)
{
    switch (errNak)
    {
    case 0x01:
        return OTI_STX_MISSING;
    case 0x02:
        return OTI_ETX_MISSING;
    case 0x03:
        return OTI_LRC_ERROR;
    case 0x04:
        return OTI_DATA_LENGTH_ERROR;
    case 0x05:
        return OTI_UNKNOWN_OPCODE;
    case 0x06:
        return OTI_TLV_ERROR;
    case 0x07:
        return OTI_DATA_LENGTH_OVERFLOW;
    case 0x08:
        return OTI_BUFFER_OVERFLOW;
    case 0x09:
        return OTI_TOO_MANY_INSTRUCTIONS;
    default:
        return OTI_NAK;
    }
}

OTI_STATUS MapErrorToStatus(BYTE err)
{
    switch (err)
    {
    case 0x01:
        return OTI_PARAM_NOT_SUPPORTED;
    case 0x02:
        return OTI_INVALID_PARAM;
    case 0x03:
        return OTI_PCD_MEM_WRITE_ERROR;
    case 0x04:
        return OTI_PCD_MEM_READ_ERROR;
    case 0x05:
        return OTI_RF_FAILURE;
    case 0x06:
        return OTI_COLLISION_ERROR;
    case 0x07:
        return OTI_RF_FAILURE_AFTER_RECOVERY;
    case 0x0A:
        return OTI_DATA_LENGTH_OVERFLOW;
    case 0x0B:
        return OTI_HERCULES_ERROR_CODE;
    case 0x0C:
        return OTI_SAM_CONTACT_FAILURE;
    case 0x0D:
        return OTI_SAM_MISSING;
    case 0x0E:
        return OTI_SAM_NOT_ACTIVE;
    case 0x0F:
        return OTI_SAM_PROTOCOL_ERROR;
    case 0x10:
        return OTI_MISSING_PARAM;
    case 0x11:
        return OTI_APPLI_DISABLED;
    case 0x12:
        return OTI_SUB_SCHEME_DISABLED;
    case 0x13:
        return OTI_INDEX_NOT_FOUND;
    case 0x14:
        return OTI_PKI_STORAGE_ERROR;
    case 0x15:
        return OTI_KEY_HASH_MISMATCH;
    case 0x16:
        return OTI_AID_LENGTH_ERROR;
    case 0x17:
        return OTI_DATA_LENGTH_ERROR;
    case 0x18:
        return OTI_EMV_TRANSACTION_TERMINATED;
    case 0x19:
        return OTI_PCD_SETUP_ERROR;
    case 0x1D:
        return OTI_SUBSCHEME_NOT_APPLICABLE;
    case 0x1E:
        return OTI_READER_BUSY;
    case 0x1F:
        return OTI_POLL_TIMEOUT;
    case 0x20:
        return OTI_BOOT_CRC_ERROR;
    case 0x21:
        return OTI_BOOT_PACKET_SEQ_ERROR;
    case 0x23:
        return OTI_BOOT_TARGET_ID_ERROR;
    case 0x24:
        return OTI_BOOT_PACKET_LEN_ERROR;
    case 0x25:
        return OTI_BOOT_FILE_TOO_BIG;
    case 0x26:
        return OTI_BOOT_STAMP_ERROR;
    case 0x27:
        return OTI_BOOL_FILE_TARGET_ERROR;
    case 0x28:
        return OTI_BOOT_KEY_VERSION_ERROR;
    case 0x29:
        return OTI_BOOT_HASH_ALGO_ERROR;
    case 0x30:
        return OTI_BOOT_META_NOT_ORIGINAL;
    case 0x31:
        return OTI_BOOT_META_MISS_MANDATORY;
    case 0x32:
        return OTI_BOOT_META_TLV_ERROR;
    case 0x33:
        return OTI_BOOT_META_LEN_ERROR;
    case 0x34:
        return OTI_BOOT_META_VERSION_ERROR;
    default:
        return OTI_ERROR;
    }
}

// end
