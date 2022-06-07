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
#include "oti_types.h"
#include "tlv.h"
#include "otilib.h"
#include "frames.h"
#include "Poll.h"

int main(int argc, char **argv)
{
    BYTE msg[900];
    MSG_BUFF msgBuf;
    msgBuf.Data = msg;
    msgBuf.Max_Length = 900;

    OTI_STATUS ret;
    BYTE selectCmd[] = {0x00, 0xA4, 0x04, 0x00, 0x0E, 0x32, 0x50, 0x41, 
                       0x59, 0x2E, 0x53, 0x59, 0x53, 0x2E, 0x44, 0x44, 
                       0x46, 0x30, 0x31, 0x00};

    MSG_BUFF databuff;
    databuff.Data = selectCmd;
    databuff.Max_Length = sizeof(selectCmd);
    databuff.Actual_Length = sizeof(selectCmd);
    BYTE buf[512];
    MSG_BUFF respBuff;
    respBuff.Data = buf;
    respBuff.Max_Length = 512;
    POLL_MODE mode = ACTIVATE_ONCE;

    BYTE track1[50];
    BYTE track2[50];
    POLL_RESP_STRUCT resp;
    resp.track1_data = track1;
    resp.track1_len = 50;
    resp.track2_data = track2;
    resp.track2_len = 50;

    if(argc != 2)
    {
        LOG_OUTPUT("Error! Invalid arguments.\r\n");
        LOG_OUTPUT("Using example: %s COM_PORT\r\n", argv[0]);
        LOG_OUTPUT("press ENTER to exit...");
        getchar();
        return 1;
    }

    if (SetReaderPort((BYTE *)argv[1]) != OTI_OK)
    {
        LOG_OUTPUT("Opening reader comm failed!");
        LOG_OUTPUT("press ENTER to exit...");
        getchar();
        return 2;
    }

    SetDebug(FALSE);    // Set to TRUE to see communication trace
    
    LOG_OUTPUT("\r\n\r\nPresent card\r\n");

    /* Example to retrieve a card's AID in transparent mode */
    BYTE enable_app_cmd[] = { 0xDf, 0x4F, 0x04, 0x10, 0x00, 0x00, 0x00 };
    RESP_MSG_STRUCT msg_resp;
    WORD bufLen = sizeof(msg);
    ExchangeCmd(OP_DO, enable_app_cmd, sizeof(enable_app_cmd), msg, &bufLen, &msg_resp, 1000);
    Poll(OP_DO, &mode, &resp, &msgBuf);

    /* Send SelectPPSE */
    ret = TransparentData(&databuff, &respBuff, &msgBuf);
    if (ret != OTI_OK)
    {
        LOG_OUTPUT("\r\n\r\nError on Transparent Data:%d\r\n",ret);
        LOG_OUTPUT("press ENTER to exit...");
        getchar();
        return ret;
    }
    /* FCI is in buf, check whether it's final response */
    if ((respBuff.Data[respBuff.Actual_Length - 2] == 0x90) && (respBuff.Data[respBuff.Actual_Length - 1] == 0x00))
    {
        LOG_OUTPUT("Card FCI: ");
        PrintBuf(respBuff.Data, (WORD)(respBuff.Actual_Length - 2));
        LOG_OUTPUT("\r\n\r\n");

        /* TODO: Extract FCI TLVs to get to AID */
    }

    LOG_OUTPUT("press ENTER to exit...");
    fflush(stdin);
    getchar();

    return OTI_OK;
}
