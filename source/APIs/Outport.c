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

#include <stdlib.h>
#include "protocol.h"
#include "frames.h"
#include "otilib.h"

/* Doxygen format:*/
/************************** OutPort() *****************************************************************************//**
* Sets the reader's OutPort control mode: define which unit (reader or host) is responsible for handling Leds/buzzer state.
*
* This API sets the reader’s OutPort control mode. 
* The "OutPort Control" mode define whether the Reader or Host is controlling the reader LEDs, Buzzer and Display:
* The host can control it through API calls, or it can be automatically handled by the reader itself.
* Note: After setting the "OutPort Control" to be controlled by the Host and performing “Load default” or “Reset”, 
* the display remains empty until the host sets it using the GraphicDisplay() APIs.
*
* @return	 OTI_STATUS				\n
			| Value					|	Description																	|
			|-----------------------|-------------------------------------------------------------------------------|
			| OTI_OK				| Function completed successfully				 								|
			| OTI_MEMORY_ERROR		| Message buffer couldn't be allocated											|
			| OTI_FAILED	    	| Command execution failed on Saturn reader			 							|
			| OTI_UNSUPPORTED		| Command not supported by Saturn reader										|
			| OTI_INVALID_DATA		| Provided data is invalid														|

* See MapErrorToStatus() for extra error messages returned by ExchangeCmd()
*
* \ingroup INTERNAL
********************************************************************************************************************/
OTI_STATUS OutPort(	BYTE opCode,	 //!< [in] OPCODE (DO, SET, GET)
					BYTE *ledState,  //!< [in/out] Combination of LEDs to turn on
					BYTE *beepState, //!< [in/out] State of the buzzer (on or off)
					PMSG_BUFF msgBuf) //!< [in] Buffer for building message
{
    RESP_MSG_STRUCT resp;
    OTI_STATUS ret;
    BYTE cmd[] = {0xDF, 0x7F, 0x00, 0x00};
    BYTE cmdLen = 3;

    WORD msgBufLen = msgBuf->Max_Length;

    if(opCode != OP_GET)
    {
        cmdLen = 4;
        cmd[2] = 0x01;
        cmd[3] |= *ledState | *beepState;
    }

    ret = ExchangeCmd(opCode, cmd, cmdLen, msgBuf->Data, &msgBufLen, &resp, SAT_DEFAULT_TIMEOUT);

    if(ret != OTI_OK)
    {
        LOG_DEBUG("OUTPORT CMD ERROR");
        return ret;
    }
    msgBuf->Actual_Length = msgBufLen;
    if(resp.succ == NULL)
    {
        if(resp.fail != NULL)
        {
            if(resp.failLen == 4)   // DF 7F 01 ERR
                ret = MapErrorToStatus(resp.fail[3]);
            else
                ret = OTI_FAILED;

            return ret;
        }
        else
        {
            return OTI_UNSUPPORTED;
        }
    }

    if(opCode == OP_GET)
    {
        if(resp.succLen == 4)   // DF 7F 01 STATE
        {
            if(ledState)
                *ledState = resp.succ[3] & 0x3E;        // LEDS + BACKLIGHT
            if(beepState)
                *beepState = resp.succ[3] & 0x01;       // BUZZER
        }
        else
            ret = OTI_INVALID_DATA;
    }

    return ret;
}// OutPort()

/* Doxygen format:*/
/************************** DoLedBeepStatus() ************************************************************************//**
* Sets the reader's Leds/Beeper state in Saturn's RAM only (DO command)
*
* This API sets a LED/Buzzer state. When it comes to the buzzer this especially means only turning it on or off. 
* Emulating a beep whill require at least two calls to this API. \n
* Changes will be effective immediately but cancelled on reader's reboot or call to LoadDefaultParams().
*
* @return	 OTI_STATUS				\n
			| Value					|	Description																	|
			|-----------------------|-------------------------------------------------------------------------------|
			| OTI_OK				| Function completed successfully				 								|
			| OTI_FAILED	    	| Command execution failed on Saturn reader			 							|

* See MapErrorToStatus() for extra error messages returned by ExchangeCmd()
*
* \ingroup UI
********************************************************************************************************************/
OTI_STATUS DoLedBeepStatus(	BYTE ledState,	//!< Combination of LEDs to turn on
							BYTE beepState, //!< State of the buzzer (on or off)
							PMSG_BUFF msgBuf) //!< [in] Buffer for building message
{
    return OutPort(OP_DO, &ledState, &beepState, msgBuf);
}//DoLedBeepStatus()

// end
