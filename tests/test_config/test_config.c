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

    Description:    Test application to load configuration to Saturn reader.
    Author:         On Track Innovations Ltd.
    Version:        4.0.00

*/
#include <stdio.h>

#include "otilib.h"
#include "protocol.h"

int main(int argc, char **argv)
{
    BYTE msg[900];
    MSG_BUFF  msgBuf;

    msgBuf.Data = msg;
    msgBuf.Max_Length = 900;

    WORD counter;
    // Structure of command: [TAG][LEN][DATA]
    // Request for reader can contain one or more command. For example: [command][command]...
    BYTE rawReqBuf[] =
    {
        // Enable Application:
        0xDf, 0x4F, 0x04, 0x3F, 0x1F, 0x08, 0x00,
        // EMV Result Tag List:
        0xDF, 0x67, 0x4B, 0x84, 0x00, 0xDF, 0x81, 0x49, 0x00, 0xDF, 0x81, 0x0B, 0x00, 0x9F,
        0x6E, 0x00, 0x9F, 0x42, 0x00, 0xDF, 0x81, 0x44, 0x00, 0xDF, 0x81, 0x29, 0x00, 0x9F,
        0x21, 0x00, 0xDF, 0x69, 0x00, 0xDF, 0x70, 0x00, 0x9F, 0x22, 0x00, 0x9F, 0x26, 0x00,
        0x82, 0x00, 0x9F, 0x34, 0x00, 0x57, 0x00, 0x9A, 0x00, 0x9F, 0x36, 0x00, 0x8E, 0x00,
        0x9F, 0x27, 0x00, 0x9F, 0x10, 0x00, 0x95, 0x00, 0xDF, 0x23, 0x00, 0x9F, 0x1F, 0x00,
        0xDF, 0x22, 0x00, 0x9B, 0x00, 0x9F, 0x37, 0x00,
        // Magstripe Result Tag List:
        0xDF, 0x66, 0x22, 0x82, 0x00, 0x9F, 0x36, 0x00, 0x8E, 0x00, 0x9F, 0x34, 0x00, 0x84,
        0x00, 0xDF, 0x69, 0x00, 0xDF, 0x23, 0x00, 0xDF, 0x22, 0x00, 0x95, 0x00, 0xDF, 0x81,
        0x44, 0x00, 0xDF, 0x81, 0x29, 0x00, 0x9F, 0x21, 0x00,
        // Terminal Capabilities:
        0x9F, 0x33, 0x0B, 0x07, 0xA0, 0x00, 0x00, 0x00, 0x04, 0x10, 0x10, 0xE0, 0x68, 0x88, // PayPass MasterCard
        0x9F, 0x33, 0x0B, 0x07, 0xA0, 0x00, 0x00, 0x00, 0x04, 0x30, 0x60, 0xE0, 0x68, 0x88, // PayPass Maestro
        0x9F, 0x33, 0x09, 0x05, 0xB0, 0x12, 0x34, 0x56, 0x78, 0xE0, 0x68, 0x88,             // PayPass Virtual
        0x9F, 0x33, 0x0B, 0x07, 0xA0, 0x00, 0x00, 0x00, 0x03, 0x10, 0x10, 0xE0, 0xF8, 0xC8, // Visa
        0x9F, 0x33, 0x0B, 0x07, 0xA0, 0x00, 0x00, 0x00, 0x03, 0x20, 0x10, 0xE0, 0xD8, 0xC8, // Visa Electron
        0x9F, 0x33, 0x0B, 0x07, 0xA0, 0x00, 0x00, 0x00, 0x03, 0x20, 0x20, 0xE0, 0xD8, 0xC8, // Visa VPay
        0x9F, 0x33, 0x0A, 0x06, 0xA0, 0x00, 0x00, 0x99, 0x99, 0x01, 0xE0, 0xD8, 0xC8,       // Visa Test
        0x9F, 0x33, 0x0B, 0x07, 0xA0, 0x00, 0x00, 0x00, 0x99, 0x90, 0x90, 0xE0, 0xD8, 0xC8, // Visa Virtual
        0x9F, 0x33, 0x0A, 0x06, 0xA0, 0x00, 0x00, 0x00, 0x25, 0x01, 0xE0, 0xC8, 0x88,       // Amex
        // Terminal Country Code: 
        0x9F, 0x1A, 0x02, 0x08, 0x40,
    };
    RESP_MSG_STRUCT resp;
    WORD bufLen = PCD_BOOT_MAX_PACKET_SIZE;
    BYTE respBuf[PCD_BOOT_MAX_PACKET_SIZE];

    if (argc != 2)
    {
        LOG_TRACE("Error! Invalid arguments.\r\n");
        LOG_TRACE("Using example: %s COM_PORT\r\n", argv[0]);
        return 1;
    }

    if (SetReaderPort((BYTE *)argv[1]) != OTI_OK)
    {
        LOG_TRACE("Opening reader comm failed!\r\n");
        return 2;
    }

    if (ExchangeCmd(OP_SET, rawReqBuf, sizeof(rawReqBuf) / sizeof(rawReqBuf[0]), respBuf, &bufLen, &resp, 0) != OTI_OK)
        LOG_OUTPUT("Executing failure.\r\n"); // One (or more) tag is failed \ un-supported.

    if (resp.succ != NULL && resp.unSupp == NULL && resp.fail == NULL)
    {
        LOG_OUTPUT("\r\nConfiguration setup finished successfully!\r\n");
    }
    else
    {
        // Check for unsupported:
        if (resp.unSupp != NULL)
        {
            LOG_OUTPUT("\r\nUnsupported (FF 02):\r\n");
            for (counter = 0; counter < resp.unSuppLen; counter++)
                LOG_OUTPUT("%02X ", resp.unSupp[counter]);
            LOG_OUTPUT("\r\n");
        }

        // Check for unsupported:
        if (resp.fail != NULL)
        {
            LOG_OUTPUT("\r\nFailed (FF 03):\r\n");
            for (counter = 0; counter < resp.failLen; ++counter)
                LOG_OUTPUT("%02X ", resp.fail[counter]);
            LOG_OUTPUT("\r\n");
        }
    }

    // Load the new configuration to RAM.
    if (LoadDefaultParams(&msgBuf) != OTI_OK)
    {
        LOG_TRACE("\r\nLoad Default Parameters failed\r\n");
        return 1;
    }
    else
    {
        LOG_OUTPUT("\r\nLoadDefaultParams() - success!\r\n");
    }

    LOG_OUTPUT("\r\nPress ENTER to exit...");
    fflush(stdin);
    getchar();

    return 0;
}
