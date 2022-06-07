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

    Description:    Test application to upload an OEF firmware file.
    Author:         On Track Innovations Ltd.
    Version:        4.0.00

*/


#include <stdio.h>
#include <string.h>
#include "otilib.h"
#include "config.h"

int main(int argc, char **argv)
{

    OTI_STATUS ret;
    BYTE msg[PCD_BOOT_MAX_BUFFER_SIZE];
    MSG_BUFF msgBuf;
    msgBuf.Data = msg;
    msgBuf.Max_Length = PCD_BOOT_MAX_BUFFER_SIZE;

    BYTE cmd[PCD_BOOT_MAX_BUFFER_SIZE];
    MSG_BUFF cmdBuf;
    cmdBuf.Data = cmd;
    cmdBuf.Max_Length = PCD_BOOT_MAX_BUFFER_SIZE;


    BYTE blk[PCD_BOOT_MAX_PACKET_SIZE];
    MSG_BUFF block;
    block.Data = blk;
    block.Max_Length = PCD_BOOT_MAX_PACKET_SIZE;

    if(argc != 3)
    {
        LOG_OUTPUT("Usage: %s filename.oef COM_PORT\r\n", argv[0]);
        return 1;
    }

    if (SetReaderPort((BYTE *)argv[2]) != OTI_OK)
    {
        LOG_OUTPUT("Opening reader comm failed!");
        return 2;
    }

    LOG_OUTPUT(">>> Loading F/W file %s to the reader...\r\n", argv[1]);
    LOG_OUTPUT(">>> Calling UploadFirmwareOTIBoot()...\r\n");

    ret = UploadFirmwareOTIBoot((BYTE *)argv[1], &block, &cmdBuf, &msgBuf);
    if(ret != OTI_OK)
    {
        LOG_TRACE(">>> Error %d\r\n", ret);
        return 1;
    }

    LOG_OUTPUT("press ENTER to exit...");
    fflush(stdin);
    getchar();

    return 0;
}
