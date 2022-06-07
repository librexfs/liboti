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

#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#include <string.h>
#include <stdio.h>
#include "otilib.h"
#include "config.h"
#include "serial.h"
#include "frames.h"
#include "tlv.h"
#include "frames.h"

#define SAT_DEFAULT_TIMEOUT 500

/* Message types for pass-through handling */
typedef enum
{
	MSG_TYPE_CMD = 0x00,
	MSG_TYPE_ACK,
	MSG_TYPE_NAK,
	MSG_TYPE_RESP = 0x80,	/* Response messages all have leftmost bit to 1 */
	MSG_TYPE_SUCCESS,
	MSG_TYPE_UNSUPPORTED,
	MSG_TYPE_FAILURE
} FRAME_TYPE;

#ifdef  __cplusplus
extern "C" {
#endif

// Send and receive functions
BYTE ComputeLrc(BYTE *buf, DWORD len);
OTI_STATUS CheckFrame(BYTE *buf, DWORD len);

OTI_STATUS SendRawCmd(BYTE opcode, BYTE *cmdData, WORD cmdLen, BYTE *msgBuf, WORD *msgBufLen);
OTI_STATUS ExchangeCmd(BYTE opcode, BYTE *cmdData, WORD cmdLen, BYTE *msgBuf, WORD *msgBufLen, RESP_MSG_STRUCT *resp, DWORD respTimeout);
OTI_STATUS BuildMsg(BYTE opcode, BYTE *cmdData, WORD cmdLen, BYTE *buf, WORD *bufLen);
OTI_STATUS SendMsg(BYTE *msg, WORD msgLen, BYTE *resBuf, WORD *resBufLen);
OTI_STATUS RecvMsg(BYTE *respBuf, WORD *respBufLen, RESP_MSG_STRUCT *respData, DWORD respTimeout);
OTI_STATUS GetFullFrame(BYTE *respBuf, WORD *respBufLen, DWORD respTimeout);
OTI_STATUS CheckResp(BYTE *respBuf, WORD *respBufLen, BYTE **bufPtr, BYTE **endPtr);
OTI_STATUS ParseResp(BYTE *bufPtr, BYTE *endPtr, RESP_MSG_STRUCT *respData);
OTI_STATUS ParseData(BYTE *data, WORD dataLen, OTI_STATUS (*callback)(FRAME_TYPE, BYTE, WORD, BYTE *, WORD));
OTI_STATUS MapErrorToStatus(BYTE err);
OTI_STATUS MapNAKToStatus(BYTE errNak);
OTI_STATUS GetResponse(BYTE *msgBuf, OTI_STATUS ret, WORD msgLen, WORD respLen, RESP_MSG_STRUCT *resp, TLV_MSG *act_emv_resp, DWORD timeout);

void SendStr(BYTE *str);    // For debugging purpose

void printbuf(BYTE *Frame, WORD FrameLen);

#ifdef  __cplusplus
}
#endif

#endif // __PROTOCOL_H__
