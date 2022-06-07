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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "otilib.h"

/* Doxygen format:*/
/************************** UploadFirmwareOTIBoot() ***************************************************************//**
*  Flash the Saturn reader with a new firmware file.
*
* This API is used to upload a new version of the firmware in the Saturn reader. 
* It uses secure firmware files (OEF files) signed by OTI.
* 
* @return	 OTI_STATUS				\n
			| Value					|	Description																	|
			|-----------------------|-------------------------------------------------------------------------------|
			| OTI_OK				| Function completed successfully.				 								|
			| OTI_FAILED	    	| Command execution failed on Saturn reader			 							|
			| OTI_MEMORY_ERROR		| Message buffer couldn't be allocated											|
			| OTI_INVALID_PARAM		| Provided data is invalid														|
			| OTI_DATA_LENGTH_ERROR | Error in command length encoding.												|

* See MapErrorToStatus() for extra error messages returned by ExchangeCmd()
*
* \ingroup GENERAL
*********************************************************************************************************************/

OTI_STATUS UploadFirmwareOTIBoot(BYTE *fileName, //!< [in] Zero-terminated string with the name of the file which contains the firmware to load.
	PMSG_BUFF block, //!< [in] Buffer with the firmware block
	PMSG_BUFF cmdBuf, //!< [in] Buffer for building command
	PMSG_BUFF msgBuf) //!< [in] Buffer for building message
{
    struct stat fstat;
    FILE *file;
    WORD blkNb;
    DWORD size, rsize, msgLen;
    OTI_STATUS ret;
    BYTE retry;

    int test = stat((char *)fileName, &fstat);

    /* Get file size */
    if(test != 0)
    {
        LOG_DEBUG("INCORRECT FILE SIZE");
        return OTI_DATA_LENGTH_ERROR;
    }
    size = fstat.st_size;

    LOG_DEBUG("File size: %lu\r\n", size);

    /* Open the file */
    file = fopen((char *)fileName, "rb");
    if(!file)
    {
        LOG_DEBUG("CANNOT OPEN FW FILE");
        return OTI_INVALID_PARAM;
    }

    LOG_DEBUG("Starting flashing process...\r\n");

    /* Check boot mode */
    LOG_DEBUG("\r\n>>> Calling GetFirmwareVersion...\r\n");
    ret = GetFirmwareVersion(1, msgBuf);
    if(ret != OTI_OK)
    {
        LOG_DEBUG("CANNOT READ FIRMWARE VERSION");
        fclose(file);
        return ret;
    }

    if(strncmp((const char*)block, "BOOT", 4) != 0)
    {
        /* Activate bootloader mode... */
        ret = CallBootLoader(msgBuf);
        if(ret != OTI_OK)
        {
            LOG_DEBUG("CALL BOOTLOADER FAILED");
            fclose(file);
            return ret;
        }

        /* Check boot mode (again) */
        LOG_DEBUG("\r\n>>> Calling GetFirmwareVersion...\r\n");
        ret = GetFirmwareVersion(1, msgBuf);
        if(ret != OTI_OK)
        {
            LOG_DEBUG("CANNOT READ FIRMWARE VERSION");
            fclose(file);
            return ret;
        }

        if (strncmp((const char*)msgBuf->Data, "BOOT", 4) != 0)
        {
            LOG_DEBUG("CALL BOOTLOADER FAILED");
            fclose(file);
            return ret;
        }
    }

    LOG_DEBUG("Flashing...\r\n");

    /* Send all packets */
    blkNb = 0;
    rsize = 0;
    while(size-rsize >= PCD_BOOT_MAX_PACKET_SIZE)
    {
        fseek(file, rsize, SEEK_SET);
        if(fread(block->Data, PCD_BOOT_MAX_PACKET_SIZE, 1, file) != 1)    // nb of items read (1 * SIZE)
        {
            fclose(file);
            return OTI_DATA_LENGTH_ERROR;
        }
        block->Actual_Length = PCD_BOOT_MAX_PACKET_SIZE;
        /* Send chunk of data (in case of error, retry PCD_BOOT_MAX_RETRIES times)*/
        retry = 0;

        do
        {
            ret = FlashPacket(blkNb, block, cmdBuf,msgBuf);
            retry++;
        } while((ret != OTI_OK) && (retry <= PCD_BOOT_MAX_RETRIES));

        if(ret != OTI_OK)
        {
            fclose(file);
            return ret;
        }

        blkNb++;
        rsize += PCD_BOOT_MAX_PACKET_SIZE;

        LOG_DEBUG("Flash: %lu%% (block %d/%d - retry=%d)\r",
               rsize*100/size, blkNb, (int)size/PCD_BOOT_MAX_PACKET_SIZE, retry);
    }
    msgLen = size-rsize;

    /* Last packet? */
    if(msgLen > 0)
    {
        /* Get last file chunk */
        fseek(file, rsize, SEEK_SET);
        if(fread(block->Data, msgLen, 1, file) != 1)
        {
            fclose(file);
            return OTI_DATA_LENGTH_ERROR;
        }
        block->Actual_Length = msgLen;

        ret = FlashPacket(blkNb, block, cmdBuf, msgBuf);
        if(ret != OTI_OK)
        {
            fclose(file);
            return ret;
        }
    }
    LOG_DEBUG("\r\n\r\nRebooting...\r\n");

    /* We should be done now, FREE our block and call main application */
    fclose(file);

    ret = RunApplication(msgBuf);
    return ret;
} // UploadFirmwareOTIBoot()

// end
