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
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "otilib.h"
#include "config.h"
#include "frames.h"
#include "protocol.h"
#include "serial.h"

/* Doxygen format:*/
/************************** SetSpeed() ***************************************************************************//**
* Sets the Redaer serial communication baud rate.
*
* To set the Host port speed, use SetReaderPortSpeed() API. \n
* If the PCD non-volatile memory content which controls the baud rate is out of scope (due to error), 
* the PCD will operate according to its firmware default baud rate (9600). \n
* Before setting PCD to a high baud rate, make sure your Host can handle that rate. \n
* Standard PC may require use of USB-RS232 adapters.
*
* @return	 OTI_STATUS				\n
			| Value					|	Description																	|
			|-----------------------|-------------------------------------------------------------------------------|
			| OTI_OK				| Function completed successfully, resp is filled 								|
			| OTI_FAILED	    	| Command execution failed on Saturn reader			 							|
			| OTI_UNSUPPORTED		| Command not supported by Saturn reader										|
			| OTI_MEMORY_ERROR		| Message buffer couldn't be allocated											|
			| OTI_INVALID_PARAM		| Provided data is invalid														|

* See MapErrorToStatus() for extra error messages returned by ExchangeCmd()
* 
* \ingroup CFG
********************************************************************************************************/
OTI_STATUS SetSpeed(BAUD_TYPE baud_rate, //!< [in] Baud rate to set on Saturn reader
	PMSG_BUFF msgBuf) //!< [in] Buffer for build message
{
    BAUD_TYPE oldBaudrate;
    OTI_STATUS ret;
    RESP_MSG_STRUCT resp;
    BYTE cmdSetSpeed[] = {0xDF, 0x05, 0x01, 0x00};      // [Serial Communication - Baud]
    BYTE cmdLoadDefaults[] = {0xDF, 0x04, 0x00};        // [Load Default Parameters]
    WORD msgBufLen = msgBuf->Max_Length;

    GetConfig();
    oldBaudrate = gConf.baud_rate;    // Save in case something goes wrong

    LOG_DEBUG("%02d SET\r\n", baud_rate);
    
    switch(baud_rate)
    {
        case BPS_9600:
            cmdSetSpeed[3] = 0x00;
            break;
        case BPS_19200:
            cmdSetSpeed[3] = 0x01;
            break;
        case BPS_38400:
            cmdSetSpeed[3] = 0x02;
            break;
        case BPS_57600:
            cmdSetSpeed[3] = 0x03;
            break;
        case BPS_115200:
            cmdSetSpeed[3] = 0x04;
            break;
        default:
            return OTI_INVALID_PARAM;
    }

    ret = ExchangeCmd(OP_SET, cmdSetSpeed, 4, msgBuf->Data, &msgBufLen, &resp, SAT_DEFAULT_TIMEOUT);
    if(ret != OTI_OK)
    {
        LOG_DEBUG("RATE CMD ERROR\r\n");
        return ret;
    }
    msgBuf->Actual_Length = msgBufLen;
    if(resp.succ == NULL)
    {
        if(resp.fail != NULL)
        {
            return OTI_FAILED;
        }
        else
        {
            return OTI_UNSUPPORTED;
        }
    }

    /* Activate new parameter. We don't call LoadDefaultParams directly since we
     * should get ACK all right, but then the reader will switch to the new baudrate 
     * and the response we get will look like garbage. If we got as far as ACK, 
     * switch the Host to the new baudrate before reading response.
     */
    /* 1. Build MSG first, reuse previous buffer */
    msgBufLen = msgBuf->Max_Length;
    ret = BuildMsg(OP_DO, cmdLoadDefaults, 3, msgBuf->Data, &msgBufLen);
    if(ret != OTI_OK)
    {
        return ret;
    }

    /* 2. Send command MSG */
    if (!Send(msgBuf->Data, msgBufLen))
    {
        LOG_DEBUG("SEND MSG ERROR\r\n");
        return OTI_COM_ERROR;
    }

    /* 3. Get and verify ACK */
    msgBufLen = msgBuf->Max_Length;
    if (!Receive(msgBuf->Data, &msgBufLen, 0))
    {
        LOG_DEBUG("GET ACK ERROR\r\n");
        return OTI_COM_ERROR;
    }

    if (CheckFrame(msgBuf->Data, msgBufLen) != OTI_OK)
    {
        LOG_DEBUG("N/ACK FRAME ERROR\r\n");
        return OTI_LRC_ERROR;
    }

    if (msgBuf->Data[3] == OP_NAK)
    {
        LOG_DEBUG("NAK\r\n");
        return MapNAKToStatus(msgBuf->Data[4]);
    }

    /* We got there, the Saturn is now probably using the new BitRate. 
     * Switch and get response. If we get an error at that point, it's bad
     * and we switch back to previous settings.
     */
    gConf.baud_rate = baud_rate;
    if(InitPort() != OTI_OK)
    {
        gConf.baud_rate = oldBaudrate;
        return OTI_CONFIG_ERROR;
    }

    /* 4. Receive response at the new baudrate and check it's complete */
    msgBufLen = msgBuf->Max_Length;
    if (!Receive(msgBuf->Data, &msgBufLen, 0))
    {
        LOG_DEBUG("GET RESP ERROR\r\n");
        return OTI_COM_ERROR;
    }

    /* If THIS is invalid, we're in trouble. */
    if ((msgBufLen < MIN_MSG_LEN) || ((msgBuf->Data[0] != MSG_STX) && (msgBuf->Data[msgBufLen - 1] != MSG_ETX)))
    {
        LOG_DEBUG("RESP INVALID\r\n");
  
        /* Restore previous settings */
        gConf.baud_rate = oldBaudrate;
        if(InitPort() != OTI_OK)
            return OTI_CONFIG_ERROR;
        else
            return OTI_INVALID_DATA;
    }

    /* Double check validity */
    if (CheckFrame(msgBuf->Data, msgBufLen) != OTI_OK)
    {
        LOG_DEBUG("RESP FRAME ERROR\r\n");
  
        /* Restore previous settings */
        gConf.baud_rate = oldBaudrate;
        if(InitPort() != OTI_OK)
            return OTI_CONFIG_ERROR;
        else
            return OTI_LRC_ERROR;
    }

    /* 5. Parse response message */
    /* We'll skip most of it. If we can read the response, it implicitely means
     * the command was successful. We'll just check that for sure. We know the command
     * and the opcode so we know exactly what the answer should look like.
     *
     * 02 0C 00 3E FF 0x 03 DF 04 00 [LRC] 03
     *
     * With 0x being either 01 (success) 02 (unsupported) or 03 (failure)
     */
    if(msgBuf->Data[5] != 0x01)
    {
        LOG_DEBUG("RESP INVALID\r\n");
  
        /* Restore previous settings */
        gConf.baud_rate = oldBaudrate;
        if(InitPort() != OTI_OK)
            return OTI_CONFIG_ERROR;
        else
            return OTI_INVALID_DATA;
    }

    LOG_DEBUG("SPEED CHANGED!\r\n");

    return OTI_OK;
}// SetSpeed()
// end
