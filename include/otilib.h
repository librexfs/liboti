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
#ifndef __OTILIB_H__
#define __OTILIB_H__            // this avoids multiple inclusions

#include "oti_types.h"
#include "platform_utils.h"
#include "SAT_Assert.h"

void LOG_TRACE(char *msg, ...);
void LOG_DEBUG(char *msg, ...);
void LOG_OUTPUT(char *msg, ...);
void LOG_OUTPUT_BUF(BYTE* py_buf, WORD w_Length);

/* Add testing/debugging code for various modules */
#ifdef DEBUG_MODE
#define OTI_DEBUG_TLV 1
#define OTI_DEBUG_PROTOCOL 1
#endif

/* OTI File names */
#define FIRMWARE_FILE   "OTIFirmware.oef"

/* Global configuration variables */
extern OTI_CONFIG_STRUCT    gConf;
extern BOOLEAN              gConfSet;

#ifdef  __cplusplus
extern "C" {
#endif

/* ALL Library function prototypes */
OTI_STATUS OTIInit(BYTE *port, BAUD_TYPE baud_rate, STOP_TYPE stop_bits, BOOLEAN debug);
OTI_STATUS TestTLV(void);
OTI_STATUS ResetPCD(PMSG_BUFF msgBuf); 
OTI_STATUS SetReaderPort(BYTE *port); 
OTI_STATUS SetReaderPortSpeed(BAUD_TYPE rate); 
OTI_STATUS SetSpeed(BAUD_TYPE baud_rate, PMSG_BUFF msgBuf);
OTI_STATUS SetCAPublicKey(OTI_CAPK_STRUCT* capk, PMSG_BUFF cmdBuf,  PMSG_BUFF msgBuf);
OTI_STATUS ActivateTransaction(ACT_EMV_STRUCT *act_emv, TLV_MSG *act_emv_resp, PMSG_BUFF msgBuf);
OTI_STATUS WaitForFC(TLV_MSG *act_emv_resp, PMSG_BUFF msgBuf);
OTI_STATUS CancelTransaction(void); 
OTI_STATUS Ping(PMSG_BUFF msgBuf);
OTI_STATUS DeleteAllCAKeys(PMSG_BUFF msgBuf); //
OTI_STATUS DeleteCAPublicKey(BYTE* rid, BYTE index, PMSG_BUFF msgBuf);
OTI_STATUS SetDebug(BOOLEAN debug);
OTI_STATUS GetSaturnConfig(OTI_CONFIG_STRUCT *config);
OTI_STATUS SelectLCDMessage(LCD_MESSAGE *index, LCDMSG_SELECT_ACT action, PMSG_BUFF msgBuf);
OTI_STATUS StoreLCDMessage(LCDMSG_SETTINGS_STRUCT *message, PMSG_BUFF cmdBuf, PMSG_BUFF msgBuf);
OTI_STATUS ChangeLCDMessage(LCDMSG_SETTINGS_STRUCT *message, PMSG_BUFF cmdBuf, PMSG_BUFF msgBuf);
OTI_STATUS GetLCDMessage(LCD_MESSAGE index, LCDMSG_SETTINGS_STRUCT *lcdResp, PMSG_BUFF cmdBuf, PMSG_BUFF msgBuf);
OTI_STATUS LoadDefaultParams(PMSG_BUFF msgBuf);
OTI_STATUS GetSerialNumber(PMSG_BUFF msgBuf);
OTI_STATUS DoOutPortControl(OTI_OUTPORT outport, PMSG_BUFF msgBuf);
OTI_STATUS SetOutPortControl(OTI_OUTPORT outport, PMSG_BUFF msgBuf);
OTI_STATUS DoLedBeepStatus(BYTE ledState, BYTE beepState, PMSG_BUFF msgBuf);
OTI_STATUS GraphicDisplay(BYTE *commands, WORD cmdsLen, PMSG_BUFF msgBuf);
OTI_STATUS GraphSetPen(BYTE **buf, WORD *bufLen, GFX_PEN pen);
OTI_STATUS GraphSetX(BYTE **buf, WORD *bufLen, BYTE xPos);
OTI_STATUS GraphSetY(BYTE **buf, WORD *bufLen, BYTE yPos);
OTI_STATUS GraphClear(BYTE **buf, WORD *bufLen);
OTI_STATUS GraphNewLine(BYTE **buf, WORD *bufLen);
OTI_STATUS GraphSetFont(BYTE **buf, WORD *bufLen, GFX_FONT font);
OTI_STATUS GraphLine(BYTE **buf, WORD *bufLen, BYTE xFrom, BYTE yFrom, BYTE xTo, BYTE yTo);
OTI_STATUS GraphBox(BYTE **buf, WORD *bufLen, BYTE xFrom, BYTE yFrom, BYTE xTo, BYTE yTo);
OTI_STATUS GraphFillBox(BYTE **buf, WORD *bufLen, BYTE xFrom, BYTE yFrom, BYTE xTo, BYTE yTo);
OTI_STATUS GraphText(BYTE **buf, WORD *bufLen, BYTE *txt);
OTI_STATUS GetFirmwareVersion(BYTE fModule, PMSG_BUFF msgBuf);
OTI_STATUS StoreUIMode(UI_MODE mode, PMSG_BUFF msgBuf);
OTI_STATUS UploadFirmwareOTIBoot(BYTE *fileName, PMSG_BUFF block, PMSG_BUFF cmdBuf, PMSG_BUFF msgBuf);
OTI_STATUS RunApplication(PMSG_BUFF msgBuf);
OTI_STATUS CallBootLoader(PMSG_BUFF msgBuf);
OTI_STATUS FlashPacket(WORD num, PMSG_BUFF packet, PMSG_BUFF cmdBuf, PMSG_BUFF msgBuf);
OTI_STATUS DoExternalDisplay(EXT_DISPLAY_MODE mode, PMSG_BUFF msgBuf);
OTI_STATUS PatternsControl(UI_PATTERN pattern, PMSG_BUFF msgBuf);
OTI_STATUS WriteDisplay(char* line1, char* line2, PMSG_BUFF cmdWriteDisply, PMSG_BUFF msgBuf);
OTI_STATUS OnlineStatus(ONLINE_STATUS status, PMSG_BUFF msgBuf);
OTI_STATUS WaitTimeExtention(PMSG_BUFF msgBuf);
OTI_STATUS TransparentData(PMSG_BUFF data, PMSG_BUFF respBuf, PMSG_BUFF msgBuf);


/* reverse compatibility for FW flashing functions */
#define UploadSecureFirmware    UploadFirmwareOTIBoot
#define UploadFirmware          UploadFirmwareYModem

/* Utility functions */
void PrintBuf(BYTE *buf, WORD bufLen);
/* Get string describing oti status code */
const char* OtiStatusStr(OTI_STATUS code);

#ifdef  __cplusplus
}
#endif

#endif // __OTILIB_H__
