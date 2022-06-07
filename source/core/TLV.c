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

PURPOSE:     TLV related functions.

PROGRAMMER:  On Track Innovations Ltd.

REVISIONS:   TBD Revision
*********************************************************************************************************************
*/

#include "tlv.h"

/* Doxygen format:*/
/**
*\ingroup TLV
*  @{
*/
BOOLEAN InitTlv(OTI_TLV *tlv)
{
    if(!tlv)
    {
        LOG_DEBUG("NULL TLV\r\n");
        return FALSE;
    }

    /* Just set all fields to safe empty values */
    tlv->rawTlv  = NULL;
    tlv->size    = 0;
    tlv->tag     = 0;
    tlv->len     = 0;
    tlv->value   = NULL;
    tlv->light   = NONE;

    return TRUE;
}

void StrTlv(OTI_TLV *tlv)
{
    DWORD i;
    
    if(!tlv)
    {
        LOG_TRACE("NULL TLV!\r\n");
        return;
    }
    
    LOG_OUTPUT("tlv.tag:  %08lu\r\n", tlv->tag);
    LOG_OUTPUT("tlv.size: %04X\r\n", tlv->size);
    LOG_OUTPUT("tlv.len:  %04X\r\n", tlv->len);
    LOG_OUTPUT("tlv.light: %s\r\n", (tlv->light ? "Yes" : "No"));
    LOG_OUTPUT("tlv.rawTlv: 0x%08X\r\n", (unsigned int)tlv->rawTlv);
    LOG_OUTPUT("tlv.value:  0x%08X\r\n", (unsigned int)tlv->value);
    LOG_OUTPUT("*tlv.rawTlv:\r\n");
    
    for (i = 0; i < tlv->size; i++)
        LOG_OUTPUT("%02X ", tlv->rawTlv[i]);
    LOG_OUTPUT("\r\n*tlv.value:\r\n");
    for (i = 0; i < tlv->len; i++)
        LOG_OUTPUT("%02X ", tlv->value[i]);
    LOG_OUTPUT("\r\nTLV Bytes:\r\n");
    for(i = 0; i < sizeof(OTI_TLV); i++)
        LOG_OUTPUT("%02X ", ((BYTE *)tlv)[i]);
    LOG_OUTPUT("\r\n");
    
}

BOOLEAN BuildTlv(DWORD tag, WORD len, BYTE *value, BYTE *buf, WORD bufLen,OTI_TLV *tlv)
{
    WORD size;
    BYTE *valPtr;

    size = TlvTagSize(tag) + TlvLengthSize(len) + len;

    /* Check provided buffer if applicable */
    if(buf != NULL && bufLen < size)
        return FALSE;


    InitTlv(tlv);
    tlv->tag = tag;
    tlv->len = len;
    tlv->size = size;

    /* Only allocate rawTlv if no buffer provided */
    if(buf == NULL)
    {
        return FALSE;
    }
    else
    {
        tlv->rawTlv = buf;
        tlv->light = TRUE; // Ensures we won't try to FREE an external pointer
    }
    /* If we got there, tlv.rawTlv is ready to be populated */
    valPtr = tlv->rawTlv;
    if(!PackTlvDataToBuffer(&valPtr, size, tag, len, value, NULL))
    {
        return FALSE;
    }

    tlv->value = tlv->rawTlv + (tlv->size - tlv->len);
    return TRUE;
    
}

BOOLEAN BuildTlvFromRaw(BYTE *data, WORD dataLen, BYTE *buf, WORD bufLen,OTI_TLV *tlv)
{

    InitTlv(tlv);

    if(buf != NULL)
    {
        tlv->light = TRUE;
        tlv->rawTlv = buf;
    }
    else
        tlv->light = FALSE;

    if (!ParseRawTlvDataToBuffer(&tlv->tag, &tlv->len, &tlv->rawTlv, &bufLen, &data, dataLen, NULL))
    {
        return FALSE;
    }

    tlv->size = bufLen;
    tlv->value = tlv->rawTlv + (tlv->size - tlv->len);

    return TRUE;
}

BOOLEAN ParseRawTlvDataToBuffer(DWORD *tag, WORD *len, BYTE **rawBuf, WORD *rawBufLen, BYTE **dataPtr, WORD dataLen, WORD *readBytes)
{
    BYTE tagLen;
    BYTE lengthLen;
    WORD size;
    BYTE *startPtr = *dataPtr;
    BYTE *endPtr = *dataPtr + dataLen;             // For bounds checking

    /* Parse fields individually, data pointer automatically updated */
    *tag = ParseTag(dataPtr, endPtr, &tagLen);
    if(*tag == TLV_TAG_ERROR)
    {
        LOG_DEBUG("RAW TAG ERROR\r\n");
        return FALSE;
    }

    *len = ParseLen(dataPtr, endPtr, &lengthLen);
    if(*len == TLV_LEN_ERROR)
    {
        LOG_DEBUG("RAW LEN ERROR\r\n");
        return FALSE;
    }

    /* Now we know enough to parse the rest of the TLV with bounds checking. */
    if(*dataPtr + *len > endPtr)
    {
        LOG_DEBUG("RAW BUF TOO SMALL\r\n");
        return FALSE;
    }

    /* Figure out actual total size from parsed data */
    size = tagLen + lengthLen + *len;

    /* If we passed a destination buffer, use it. Otherwise, allocate */
    if(*rawBuf != NULL)
    {
        if(*rawBufLen < size)
            return FALSE;
    }
    else
    {
        return FALSE;
    }

    /* If the raw Data provided is static and can be used safely, it's possible
     * to point to it directly from the TLV. As such, providing the same buffer
     * for parsing AND storing is legal, if you know what you're doing.
     */
    if(*rawBuf != startPtr)
    {
        memmove(*rawBuf, startPtr, size);
    }
    *rawBufLen = size;


    /* Update pointer to point right after our value */
    *dataPtr += *len;

    /* If applicable store number of read bytes directly */
    if(readBytes)
        *readBytes = size;

    return TRUE;
}

BOOLEAN PackTlvDataToBuffer(BYTE **dstBuf, WORD dstBufLen, DWORD tag, WORD len, BYTE *value, WORD *writtenBytes)
{
    BYTE *endPtr;
    BYTE packed = 0;

    if(writtenBytes)
        *writtenBytes = 0;

    endPtr = *dstBuf + dstBufLen;

    if(!PackTag(tag, dstBuf, endPtr, &packed))
        return FALSE;
    if(writtenBytes)
        *writtenBytes += packed;

    packed = 0;
    if(!PackLen(len, dstBuf, endPtr, &packed))
        return FALSE;
    if(writtenBytes)
        *writtenBytes += packed;

    memcpy(*dstBuf, value, len);
    (*dstBuf) += len;
    if(writtenBytes)
        *writtenBytes += len;

    return TRUE;
}

BYTE TlvLengthSize(WORD len)
{
    BYTE ret;

    if(len < 0x7F)          // From 0x00 to 0x7F
        ret = 1;
    else if(len < 0x0100)   // From 0x81 0x80 to 0x81 0xFF
        ret = 2;
    else                    // From 0x82 0x01 0x00 to 0x82 0xFF 0xFF
        ret = 3;

    return ret;
}

BYTE TlvTagSize(DWORD tag)
{
    BYTE ret;

    if(tag < 0x0100)
        ret = 1;
    else if(tag < 0x010000)
        ret = 2;            // No tag on more than two bytes in OTI specs
    else if(tag < 0x01000000)
        ret = 3;
    else
        ret = 4;

    return ret;
}

DWORD ParseTag(BYTE **dataPtr, BYTE *endPtr, BYTE *readBytes)
{
    DWORD nTag = 0x00000000;
    BYTE nbMoreBytes = 0;

    /* Sanity check */
    if(readBytes) // Initialize output parameter
        *readBytes = 0;

    if(*dataPtr >= endPtr)
        return TLV_TAG_ERROR;

    /* Read and save first byte */
    nTag = **dataPtr;
    (*dataPtr)++;
    if(readBytes)
        (*readBytes)++;

    if ((nTag & 0x1F) == 0x1F) // More bytes follow, like 9F 26
    {
        /* Read and save next byte */
        if(*dataPtr < endPtr)
        {
            nTag = (nTag << 8) + **dataPtr;
            (*dataPtr)++;
            if(readBytes)
                (*readBytes)++;

            while((nTag & 0x80) == 0x80)
            {
                if(nbMoreBytes == 2)
                    return TLV_TAG_ERROR;
                nbMoreBytes++;
                nTag = (nTag << 8) + **dataPtr;
                (*dataPtr)++;
                if(readBytes)
                    (*readBytes)++;
            }
        }
        else
            return TLV_TAG_ERROR;
    }

    return nTag;
}

WORD ParseLen(BYTE **dataPtr, BYTE *endPtr, BYTE *readBytes)
{
    WORD nLen = 0;
    WORD i;
    WORD nBytesInLength;

    /* Sanity check */
    if(readBytes) // Initialize output parameter
        *readBytes = 0;
    if(*dataPtr > endPtr)
        return TLV_LEN_ERROR;

    if ((**dataPtr & 0x80) == 0x80) // like 82 01 3A
    {
        nBytesInLength = (**dataPtr & 0x7F);

        if(*dataPtr + nBytesInLength > endPtr)
            return TLV_LEN_ERROR;

        (*dataPtr)++;
        if(readBytes)
            (*readBytes)++;
        
        for (i = 0; i < nBytesInLength; i++)
        {
            nLen = (nLen * 0x100) + **dataPtr;
            (*dataPtr)++;
            if(readBytes)
                (*readBytes)++;
        }
    }
    else // Like 13
    {
        nLen = **dataPtr;
        (*dataPtr)++;
        if(readBytes)
            (*readBytes)++;
    }

    return nLen;
}

BOOLEAN PackTag(DWORD tag, BYTE **bufPtr, BYTE *endPtr, BYTE *writtenBytes)
{
    BYTE sTag;
    WORD i;

    sTag = TlvTagSize(tag);
    if(*bufPtr + sTag > endPtr)
        return FALSE;

    /* Set tag bytes from tag's LSB to MSB */
    for(i = sTag; i > 0; i--)
    {
        *(*bufPtr + i - 1) = (BYTE)(tag & 0xFF);
        tag = tag >> 8;
    }

    /* Update buffer pointer to make it point just after tag's last byte */
    *bufPtr += sTag;
    if(writtenBytes)
        *writtenBytes = sTag;

    return TRUE;
}

BOOLEAN PackLen(WORD len, BYTE **bufPtr, BYTE *endPtr, BYTE *writtenBytes)
{
    BYTE sLen;
    WORD i;

    sLen = TlvLengthSize(len);
    if(writtenBytes)
        *writtenBytes = 0;
    if(*bufPtr + sLen > endPtr)
        return FALSE;

    /* Now pack bytes in buffer starting by MSB */
    if(sLen > 1) {
        **bufPtr = (0x80 | (sLen - 1));
        (*bufPtr)++;
        if(writtenBytes)
            (*writtenBytes)++;
        sLen--;      // number of bytes remaining in LEN field to pack
    }

    for(i = sLen; i > 0; i--) {
        *(*bufPtr + i - 1) = (BYTE)(len & 0xFF);
        len = len >> 8;
    }

    /* Update buffer pointer to point right after LEN bytes */
    *bufPtr += sLen;
    if(writtenBytes)
        *writtenBytes += sLen;

    return TRUE;
}

BOOLEAN ExtractTLV(BYTE *data, WORD dataLen, BOOLEAN shallow, WORD *path, BYTE pathLen,OTI_TLV *tlv)
{
    BYTE  found = FALSE;
    BYTE *startTlv;
    BYTE *bufPtr;
    BYTE *endTlv;            /* For bounds checking */
    BYTE  pathIdx = 0;
    DWORD tag;
    WORD  len;
    WORD  size;

    if(!data)
        return FALSE;

    endTlv   = data + dataLen;
    startTlv = data;
    bufPtr   = data;
    
    /* Check each tag against the current entry of the path. If it doesn't
       match, skip to next tag. Otherwise, parse data and get to next item
       in path.
     */
    do
    {
        /* Parse tag and len */
        tag = ParseTag(&bufPtr, endTlv, NULL);
        len = ParseLen(&bufPtr, endTlv, NULL);

        /* Match tag against current entry in path */
        if(tag == path[pathIdx])
        {
            /* If this is the end of our path, we got our TLV */
            if(pathIdx == (pathLen - 1))
            {
                found = TRUE;

                /* Exit loop to create TLV */
                break;
            }
            else
            {
                /* Move to next entry in path and loop */
                pathIdx++;
            }
        }
        else
        {
            /* Wrong tag, try skipping to the next TLV */
            bufPtr += len;
        }

        /* At this point, bufPtr points either to the next TLV or the data in
           the current TLV. Save that position in startTlv now.
         */
        startTlv = bufPtr;
        
    } while((bufPtr < endTlv) && (pathIdx < pathLen) && (!found));
    
    /* We're at the end of our data array or we found our TLV */
    if(found)
    {
        size = (bufPtr + len) - startTlv;
        OTI_TLV tlv;
        if(shallow)
            return BuildTlvFromRaw(startTlv, size, startTlv, size, &tlv);
        else
            return BuildTlvFromRaw(startTlv, size, NULL, 0, &tlv);
    }
    else
    {
        return FALSE;
    }
}


/* Testing TLV functions */
OTI_STATUS TestTLV(void)
{
#ifdef OTI_DEBUG_TLV
    OTI_TLV tlv;
    BOOLEAN pass = FALSE;
    BYTE buf[0x120];
    //BYTE cmd[] = {0x02, 0x09, 0x00, 0x3E, 0xDF, 0x19, 0x00, 0xF3, 0x03};
    BYTE tlvData[] = { 0x01, 0x02, 0x03, 0x04 };
    BYTE tlv01[] = { 0xFC, 0x04, 0x01, 0x02, 0x03, 0x04 };
    BYTE tlv02[] = { 0xDF, 0x19, 0x04, 0x01, 0x02, 0x03, 0x04 };
    BYTE tlv03[] = { 0xFC, 0x82, 0x01, 0x20 }; // Header only
    BYTE tlv04[] = { 0xDF, 0x19, 0x82, 0x01, 0x20 }; // Header only
    BYTE tlv05[] = { 0xFC, 0x82, 0x01, 0x00 }; // Header only
    BYTE tlv06[] = { 0xDF, 0x19, 0x82, 0x01, 0x00 }; // Header only
    BYTE tlv07[] = { 0xDF, 0x81, 0x09, 0x04, 0x01, 0x02, 0x03, 0x04 };
    BYTE longTlvData[0x100];
    BYTE longRawTlv[0x105];

    memset(buf, 0x11, sizeof(buf));
    memset(longTlvData, 0x22, sizeof(longTlvData));
    memset(longRawTlv, 0x33, sizeof(longRawTlv));

    LOG_DEBUG("OTI LIBTLV UNIT TESTING\r\n-----------------------\r\n");

    /* 1. TLV from short tag, short length and value, allocated buffer */
    pass = FALSE;
    if(BuildTlv(0xFC, 0x04, tlvData, NULL, 0,&tlv))
    {
        pass = ((tlv.len == 4)
            && (tlv.light == FALSE)
            && (tlv.size == 6)
            && (tlv.tag == 0xFC)
            && (memcmp(tlv.value, tlvData, sizeof(tlvData)) == 0)
            && (memcmp(tlv.rawTlv, tlv01, sizeof(tlv01)) == 0));
    }
    if (pass)
        LOG_DEBUG(" 1.[T1|L1|V4|D]     :OK \r\n");
    else
        LOG_DEBUG(" 1.[T1|L1|V4|D]     :ERR\r\n");

    /* 2. TLV from long tag, short length and value, allocated buffer */
    pass = FALSE;
    if(BuildTlv(0xDF19, 0x04, tlvData, NULL, 0,&tlv))
    {
        pass = ((tlv.len == 4)
            && (tlv.light == FALSE)
            && (tlv.size == 7)
            && (tlv.tag == 0xDF19)
            && (memcmp(tlv.value, tlvData, sizeof(tlvData)) == 0)
            && (memcmp(tlv.rawTlv, tlv02, sizeof(tlv02)) == 0));
    }
    if (pass)
        LOG_DEBUG(" 2.[T2|L1|V4|D]     :OK \r\n");
    else
        LOG_DEBUG(" 2.[T2|L1|V4|D]     :ERR\r\n");

    /* 3. TLV from short tag, long length and value, allocated buffer */
    pass = FALSE;

    if(BuildTlv(0xFC, 0x120, buf, NULL, 0,&tlv))
    {
        //StrTlv(tlv);

        pass = ((tlv.len == 0x120)
            && (tlv.light == FALSE)
            && (tlv.size == 0x124)
            && (tlv.tag == 0xFC)
            && (memcmp(tlv.value, buf, sizeof(buf)) == 0)
            && (memcmp(tlv.rawTlv, tlv03, sizeof(tlv03)) == 0));
    }
    if (pass)
        LOG_DEBUG(" 3.[T1|L3|V288|D]   :OK \r\n");
    else
        LOG_DEBUG(" 3.[T1|L3|V288|D]   :ERR\r\n");

    /* 4. TLV from long tag, long length and value, allocated buffer */
    pass = FALSE;
    
    if(BuildTlv(0xDF19, 0x120, buf, NULL, 0,&tlv))
    {
        pass = ((tlv.len == 0x120)
            && (tlv.light == FALSE)
            && (tlv.size == 0x125)
            && (tlv.tag == 0xDF19)
            && (memcmp(tlv.value, buf, sizeof(buf)) == 0)
            && (memcmp(tlv.rawTlv, tlv04, sizeof(tlv04)) == 0));
    }
    if (pass)
        LOG_DEBUG(" 4.[T2|L3|V288|D]   :OK \r\n");
    else
        LOG_DEBUG(" 4.[T2|L3|V288|D]   :ERR\r\n");

    /* 5. TLV from short tag, short length and value, static buffer */
    pass = FALSE;
    memset(buf, 0x11, sizeof(buf));

    if(BuildTlv(0xFC, 0x04, tlvData, buf, sizeof(buf),&tlv))
    {
        pass = ((tlv.len == 4)
            && (tlv.light == TRUE)
            && (tlv.size == 6)
            && (tlv.tag == 0xFC)
            && (tlv.rawTlv == buf)
            && (memcmp(tlv.value, tlvData, sizeof(tlvData)) == 0)
            && (memcmp(tlv.rawTlv, tlv01, sizeof(tlv01)) == 0));
    }
    if (pass)
        LOG_DEBUG(" 5.[T1|L1|V4|S]     :OK \r\n");
    else
        LOG_DEBUG(" 5.[T1|L1|V4|S]     :ERR\r\n");

    /* 6. TLV from long tag, short length and value, static buffer */
    pass = FALSE;
    memset(buf, 0x11, sizeof(buf));

    if(BuildTlv(0xDF19, 0x04, tlvData, buf, sizeof(buf),&tlv))
    {
        pass = ((tlv.len == 4)
            && (tlv.light == TRUE)
            && (tlv.size == 7)
            && (tlv.tag == 0xDF19)
            && (tlv.rawTlv == buf)
            && (memcmp(tlv.value, tlvData, sizeof(tlvData)) == 0)
            && (memcmp(tlv.rawTlv, tlv02, sizeof(tlv02)) == 0));
    }
    if (pass)
        LOG_DEBUG(" 6.[T2|L1|V4|S]     :OK \r\n");
    else
        LOG_DEBUG(" 6.[T2|L1|V4|S]     :ERR\r\n");

    /* 7. TLV from short tag, long length and value, static buffer */
    pass = FALSE;
    memset(buf, 0x11, sizeof(buf));
     
    if(BuildTlv(0xFC, 0x100, longTlvData, buf, sizeof(buf),&tlv))
    {
        pass = ((tlv.len == 0x100)
            && (tlv.light == TRUE)
            && (tlv.size == 0x104)
            && (tlv.tag == 0xFC)
            && (tlv.rawTlv == buf)
            && (memcmp(tlv.value, longTlvData, sizeof(longTlvData)) == 0)
            && (memcmp(tlv.rawTlv, tlv05, sizeof(tlv05)) == 0));
    }
    if (pass)
        LOG_DEBUG(" 7.[T1|L3|V256|S]   :OK \r\n");
    else
        LOG_DEBUG(" 7.[T1|L3|V256|S]   :ERR\r\n");

    /* 8. TLV from long tag, long length and value, static buffer */
    pass = FALSE;
    memset(buf, 0x11, sizeof(buf));
     
    if(BuildTlv(0xDF19, 0x100, longTlvData, buf, sizeof(buf),&tlv))
    {
        pass = ((tlv.len == 0x100)
            && (tlv.light == TRUE)
            && (tlv.size == 0x105)
            && (tlv.tag == 0xDF19)
            && (tlv.rawTlv == buf)
            && (memcmp(tlv.value, longTlvData, sizeof(longTlvData)) == 0)
            && (memcmp(tlv.rawTlv, tlv06, sizeof(tlv06)) == 0));
    }
    if (pass)
        LOG_DEBUG(" 8.[T2|L3|V256|S]   :OK \r\n");
    else
        LOG_DEBUG(" 8.[T2|L3|V256|S]   :ERR\r\n");

    /* 9. TLV from raw bytes, short tag, short length and value, allocated buffer */
    pass = FALSE;
    
    if (BuildTlvFromRaw(tlv01, 0x06, NULL, 0,&tlv))
    {
        pass = ((tlv.len == 4)
            && (tlv.light == FALSE)
            && (tlv.size == 6)
            && (tlv.tag == 0xFC)
            && (memcmp(tlv.value, tlvData, sizeof(tlvData)) == 0)
            && (memcmp(tlv.rawTlv, tlv01, sizeof(tlv01)) == 0));
    }
    if (pass)
        LOG_DEBUG(" 9.[R|T1|L1|V4|D]   :OK \r\n");
    else
        LOG_DEBUG(" 9.[R|T1|L1|V4|D]   :ERR\r\n");

    /* 10. TLV from raw bytes, long tag, short length and value, allocated buffer */
    pass = FALSE;
    
    if (BuildTlvFromRaw(tlv02, 0x07, NULL, 0,&tlv))
    {
        pass = ((tlv.len == 4)
            && (tlv.light == FALSE)
            && (tlv.size == 7)
            && (tlv.tag == 0xDF19)
            && (memcmp(tlv.value, tlvData, sizeof(tlvData)) == 0)
            && (memcmp(tlv.rawTlv, tlv02, sizeof(tlv02)) == 0));
    }
    if (pass)
        LOG_DEBUG("10.[R|T2|L1|V4|D]   :OK \r\n");
    else
        LOG_DEBUG("10.[R|T2|L1|V4|D]   :ERR\r\n");

    /* 11. TLV from short tag, long length and value, allocated buffer */
    pass = FALSE;
    memset(longRawTlv, 0x33, sizeof(longRawTlv));
    memcpy(longRawTlv, tlv05, sizeof(tlv05));
     
    if (BuildTlvFromRaw(longRawTlv, 0x104, NULL, 0,&tlv))
    {
        pass = ((tlv.len == 0x100)
            && (tlv.light == FALSE)
            && (tlv.size == 0x104)
            && (tlv.tag == 0xFC)
            && (memcmp(tlv.value, &longRawTlv[4], 0x100) == 0)
            && (memcmp(tlv.rawTlv, longRawTlv, 0x104) == 0));
    }

    if (pass)
        LOG_DEBUG("11.[R|T1|L3|V256|D] :OK \r\n");
    else
        LOG_DEBUG("11.[R|T1|L3|V256|D] :ERR\r\n");

    /* 12. TLV from raw bytes, long tag, long length and value, allocated buffer */
    pass = FALSE;
    memset(longRawTlv, 0x33, sizeof(longRawTlv));
    memcpy(longRawTlv, tlv06, sizeof(tlv06));
    if (BuildTlvFromRaw(longRawTlv, 0x105, NULL, 0, &tlv))
    {
        pass = ((tlv.len == 0x100)
            && (tlv.light == FALSE)
            && (tlv.size == 0x105)
            && (tlv.tag == 0xDF19)
            && (memcmp(tlv.value, &longRawTlv[5], 0x100) == 0)
            && (memcmp(tlv.rawTlv, longRawTlv, 0x105) == 0));
    }
    if (pass)
        LOG_DEBUG("12.[R|T2|L3|V256|D] :OK \r\n");
    else
        LOG_DEBUG("12.[R|T2|L3|V256|D] :ERR\r\n");

    /* 13. TLV from raw bytes, short tag, short length and value, static buffer */
    pass = FALSE;
    memset(buf, 0x11, sizeof(buf));

    if (BuildTlvFromRaw(tlv01, 0x06, buf, sizeof(buf),&tlv))
    {
        pass = ((tlv.len == 4)
            && (tlv.light == TRUE)
            && (tlv.size == 6)
            && (tlv.tag == 0xFC)
            && (tlv.rawTlv == buf)
            && (memcmp(tlv.value, tlvData, sizeof(tlvData)) == 0)
            && (memcmp(tlv.rawTlv, tlv01, sizeof(tlv01)) == 0));
    }
    if (pass)
        LOG_DEBUG("13.[R|T1|L1|V4|S]   :OK \r\n");
    else
        LOG_DEBUG("13.[R|T1|L1|V4|S]   :ERR\r\n");

    /* 14. TLV from raw bytes, long tag, short length and value, static buffer */
    pass = FALSE;
    memset(buf, 0x11, sizeof(buf));
    if (BuildTlvFromRaw(tlv02, 0x07, buf, sizeof(buf),&tlv))
    {
        pass = ((tlv.len == 4)
            && (tlv.light == TRUE)
            && (tlv.size == 7)
            && (tlv.tag == 0xDF19)
            && (tlv.rawTlv == buf)
            && (memcmp(tlv.value, tlvData, sizeof(tlvData)) == 0)
            && (memcmp(tlv.rawTlv, tlv02, sizeof(tlv02)) == 0));
    }
    if (pass)
        LOG_DEBUG("14.[R|T2|L1|V4|S]   :OK \r\n");
    else
        LOG_DEBUG("14.[R|T2|L1|V4|S]   :ERR\r\n");

    /* 15. TLV from short tag, long length and value, allocated buffer */
    pass = FALSE;
    memset(buf, 0x11, sizeof(buf));
    memset(longRawTlv, 0x33, sizeof(longRawTlv));
    memcpy(longRawTlv, tlv05, sizeof(tlv05));

    if (BuildTlvFromRaw(longRawTlv, 0x104, buf, sizeof(buf),&tlv))
    {
        pass = ((tlv.len == 0x100)
            && (tlv.light == TRUE)
            && (tlv.size == 0x104)
            && (tlv.tag == 0xFC)
            && (tlv.rawTlv == buf)
            && (memcmp(tlv.value, &longRawTlv[4], 0x100) == 0)
            && (memcmp(tlv.rawTlv, longRawTlv, 0x104) == 0));
    }

    if (pass)
        LOG_DEBUG("15.[R|T1|L3|V256|S] :OK \r\n");
    else
        LOG_DEBUG("15.[R|T1|L3|V256|S] :ERR\r\n");

    /* 16. TLV from raw bytes, long tag, long length and value, allocated buffer */
    pass = FALSE;
    memset(buf, 0x11, sizeof(buf));
    memset(longRawTlv, 0x33, sizeof(longRawTlv));
    memcpy(longRawTlv, tlv06, sizeof(tlv06));
    
    if (BuildTlvFromRaw(longRawTlv, 0x105, buf, sizeof(buf),&tlv))
    {
        pass = ((tlv.len == 0x100)
            && (tlv.light == TRUE)
            && (tlv.size == 0x105)
            && (tlv.tag == 0xDF19)
            && (tlv.rawTlv == buf)
            && (memcmp(tlv.value, &longRawTlv[5], 0x100) == 0)
            && (memcmp(tlv.rawTlv, longRawTlv, 0x105) == 0));
    }
    if (pass)
        LOG_DEBUG("16.[R|T2|L3|V256|S] :OK \r\n");
    else
        LOG_DEBUG("16.[R|T2|L3|V256|S] :ERR\r\n");

    /* 17. TLV from raw bytes, very long tag, short length and value, static buffer */
    pass = FALSE;
    memset(buf, 0x11, sizeof(buf));
    
    if (BuildTlvFromRaw(tlv07, 0x08, buf, sizeof(buf),&tlv))
    {
        pass = ((tlv.len == 4)
            && (tlv.light == TRUE)
            && (tlv.size == 8)
            && (tlv.tag == 0xDF8109)
            && (tlv.rawTlv == buf)
            && (memcmp(tlv.value, tlvData, sizeof(tlvData)) == 0)
            && (memcmp(tlv.rawTlv, tlv07, sizeof(tlv07)) == 0));
    }
    if (pass)
        LOG_DEBUG("17.[R|T3|L1|V4|S]   :OK \r\n");
    else
        LOG_DEBUG("17.[R|T3|L1|V4|S]   :ERR\r\n");

    LOG_DEBUG("\r\nDone.\r\n");
#else
    LOG_DEBUG("TLV TESTING DISABLED");
#endif //OTI_DEBUG_TLV

    return OTI_OK;
}
//Doxygen end of group: 
//*@}

// end
