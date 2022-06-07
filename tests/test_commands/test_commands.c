/**
OTI HostPCD Protocol C Library

**********************************************************************
**        Copyright (c) 2008-2015 On Track Innovations Ltd.         **
** This computer program includes confidential, proprietary         **
** information and is a trade secret of On Track Innovations Ltd.   **
** All use, disclosure, and/or reproduction is prohibited           **
** unless expressly authorized in writing.                          **
**                    All rights reserved.                          **
**********************************************************************

Description:    Test application that calls some of the Library's functions.
Author:         On Track Innovations Ltd.
Version:        4.0.00

Notes:          If the library was compiled with -DDEBUG_MODE, this will  display some test results.
Otherwise, a mere informational message should be displayed.

*/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "platform_utils.h"
#include "oti_types.h"
#include "otilib.h"

#define MESG_LEN 900
static BYTE msg[MESG_LEN];
static BYTE cmd[MESG_LEN];

void printbuffer(BYTE *buf, WORD bufLen)
{
    WORD i;

    for (i = 0; i < bufLen; i++)
    {
        LOG_OUTPUT("%02X ", buf[i]);
    }

    LOG_OUTPUT("\r\n");
}

void SetBuffers(PMSG_BUFF msgBuf, PMSG_BUFF cmdBuf)
{
    msgBuf->Data = msg;
    msgBuf->Max_Length = MESG_LEN;
    cmdBuf->Data = cmd;
    cmdBuf->Max_Length = MESG_LEN;
}


int main(int argc, char **argv)
{
    MSG_BUFF msgBuf;
    MSG_BUFF cmdBuf;
    BYTE ledStates[] = { OUT_LED1_ON,
        OUT_LED2_ON,
        OUT_LED3_ON,
        OUT_LED4_ON,
        OUT_LED3_ON,
        OUT_LED2_ON };
    BYTE i, j;
    LCD_MESSAGE msgIdx;
    LCDMSG_SETTINGS_STRUCT    lcdMsg;

    if (argc != 2)
    {
        LOG_TRACE("Error! Invalid arguments.\r\n");
        LOG_TRACE("Using example: %s COM_PORT\r\n", argv[0]);
        return 1;
    }


    if (SetReaderPort((BYTE *)argv[1]) != OTI_OK)
    {
        LOG_TRACE("Opening reader comm failed!");
        return 2;
    }

    SetDebug(FALSE);

    LOG_OUTPUT(">>> Calling GetSerialNumber...\r\n");
    SetBuffers(&msgBuf, &cmdBuf);
    if (GetSerialNumber(&msgBuf) == OTI_OK)
    {
        LOG_DEBUG(">>> Serial Number: ");
        printbuffer(msgBuf.Data, msgBuf.Actual_Length);
    }
    else
        LOG_TRACE("Failed!\r\n");


    LOG_OUTPUT("\r\n>>> Calling GetFirmwareVersion...\r\n");
    SetBuffers(&msgBuf, &cmdBuf);
    for (i = 0; i < 2; i++)
    {
        if (GetFirmwareVersion(i, &msgBuf) == OTI_OK)
        {
            msgBuf.Data[msgBuf.Actual_Length] = '\0';
            LOG_DEBUG(">>> Module index %d - Firmware Version: %s\r\n", i, msgBuf.Data);
        }
        else
            LOG_TRACE("Failed!\r\n");
    }

    LOG_OUTPUT("\r\n>>> Changing UI mode...\r\n");
    SetBuffers(&msgBuf, &cmdBuf);
    if (StoreUIMode(EUROPE_UI, &msgBuf) == OTI_OK)
        LOG_DEBUG(">>> UI Changed...\r\n");
    else
        LOG_TRACE("Failed!\r\n");

    LOG_OUTPUT("\r\n>>> Start LEDs and Buzzer predefined pattern... ");
    
    SetBuffers(&msgBuf, &cmdBuf);
    DoOutPortControl(OUT_HOST, &msgBuf);

    if (PatternsControl(LED1_BLINK_PATTERN, &msgBuf) == OTI_OK)
        LOG_OUTPUT("Done!\r\n");
    else
        LOG_OUTPUT("Failed!\r\n");

    SystemSleep(2000);

    LOG_OUTPUT("\r\n>>> Manuall controlling LEDs and Buzzer...");
    /* Don't forget to take over Outport Control or the SetLedBeepStatus
     * and SelectLCDMessage commands will not work.
     */
    SetBuffers(&msgBuf, &cmdBuf);
    for (i = 0; i < 1; i++)
    {
        for (j = 0; j < sizeof(ledStates); j++)
        {
            DoLedBeepStatus(ledStates[j], 1, &msgBuf); // Turn led[j] and start beeping on buzzer.
            SystemSleep(100);
        }
    }

    SetBuffers(&msgBuf, &cmdBuf);
    DoLedBeepStatus(0, 0, &msgBuf); // Turn off all LEDS and shutdown buzzer.
    LOG_OUTPUT("Done!\r\n");

    /* Change idle display and include a special symbol (Euro) */

    LOG_OUTPUT("\r\n>>> Store & present message...");

    msgIdx = IDLE_MSG;
    lcdMsg.index = msgIdx;
    lcdMsg.yCenter = TRUE;
    lcdMsg.setLines = LCD_LINE_ALL;
    for (i = 0; i < 3; i++)
        lcdMsg.format[i] = CENTER_X | FONT_EN_MEDIUM;

    /* Only set one line, leave the two others empty */
    srand((unsigned)time(NULL));

    sprintf((char*)lcdMsg.L1, "Save \x24%d today", rand() % 30);

    lcdMsg.L2[0] = '\0';
    lcdMsg.L3[0] = '\0';

    SetBuffers(&msgBuf, &cmdBuf);
    StoreLCDMessage(&lcdMsg, &cmdBuf, &msgBuf);
    SetBuffers(&msgBuf, &cmdBuf);
    DoOutPortControl(OUT_HOST, &msgBuf); // Mandatory in order to allow the host to control of the display directly! 
    SetBuffers(&msgBuf, &cmdBuf);
    SelectLCDMessage(&msgIdx, LCD_DISPLAY, &msgBuf);

    LOG_OUTPUT("Done!\r\n");

    SystemSleep(2000);

    LOG_OUTPUT("\r\n>>> Manually change message...");

    SetBuffers(&msgBuf, &cmdBuf);
    WriteDisplay("Hello", "World!", &cmdBuf, &msgBuf); // Requires character display, does not work well on SATURN 6000 graphic display!.

    LOG_OUTPUT("\r\npress ENTER to exit...");
    fflush(stdin);
    getchar();

    return 0;
}
