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
/************************** DoOutPortControl() *************************************************************************//**
* Sets the reader's OutPort control mode: define which unit (reader or host) is responsible for handling Leds/Buzzer/LCD state.
*
* The "OutPort Control" mode define whether the Reader or Host is controlling the reader LEDs, Buzzer and Display: \n
* - By default, the reader handles its display, leds and buzzer automatically (this is PCD mode). 
* - It's possible to let the host, to which the reader is connected, to handle display/leds/buzzer (this is the Host mode).  
* 
* Notes: 
* - After setting the "OutPort Control" to be controlled by the Host and performing LoadDefaultParams(),
* the display remains empty until the host sets it using the GraphicDisplay() APIs.
* - This comamnd changes the settings temporarily (in RAM), use SetOutPortControl() API if you wish to set it permanently (in EEPROM).
*
* @return	 OTI_STATUS				\n
			| Value					|	Description																	|
			|-----------------------|-------------------------------------------------------------------------------|
			| OTI_OK				| Function completed successfully				 								|
			| OTI_MEMORY_ERROR		| Message buffer couldn't be allocated											|
			| OTI_FAILED	    	| Command execution failed on Saturn reader			 							|
			| OTI_UNSUPPORTED		| Command not supported by Saturn reader										|

* See MapErrorToStatus() for extra error messages returned by ExchangeCmd()
*
* \ingroup UI
********************************************************************************************************************/

OTI_STATUS DoOutPortControl(OTI_OUTPORT outport, //!< [in] Outport Mode (PCD, Host or Host-LCD)
	PMSG_BUFF msgBuf) //!< [in] Buffer for building message
{
    RESP_MSG_STRUCT resp;
    OTI_STATUS ret;
    BYTE cmd[] = {0xDF, 0x0A, 0x01, 0x00};

    WORD msgBufLen = msgBuf->Max_Length;

    cmd[3] = (BYTE)outport;

    ret = ExchangeCmd(OP_DO, cmd, sizeof(cmd), msgBuf->Data, &msgBufLen, &resp, 1000);

    if(ret != OTI_OK)
    {
        LOG_DEBUG("SET OUTPORT CTL CMD ERROR");
        return ret;
    }

    if(resp.succ == NULL)
    {
        if(resp.fail != NULL)
        {
            if(resp.failLen == 4)   // DF 0A 01 ERR
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

    return ret;
}// DoOutPortControl()

/* Doxygen format:*/
/************************** SetOutPortControl() *************************************************************************//**
* Sets the reader's OutPort control mode: define which unit (reader or host) is responsible for handling Leds/Buzzer/LCD state.
*
* The "OutPort Control" mode define whether the Reader or Host is controlling the reader LEDs, Buzzer and Display: \n
* - By default, the reader handles its display, leds and buzzer automatically (this is PCD mode). 
* - It's possible to let the host, to which the reader is connected, to handle display/leds/buzzer (this is the Host mode).  
* 
* Notes: 
* - After setting the "OutPort Control" to be controlled by the Host and performing LoadDefaultParams() or ResetPCD(),
* the display remains empty until the host sets it using the GraphicDisplay() APIs.
* - This comamnd changes the settings in the Reader EEPROM, use DoOutPortControl() API if you wish to set it temporarily (in RAM).
*
* @return	 OTI_STATUS				\n
			| Value					|	Description																	|
			|-----------------------|-------------------------------------------------------------------------------|
			| OTI_OK				| Function completed successfully				 								|
			| OTI_MEMORY_ERROR		| Message buffer couldn't be allocated											|
			| OTI_FAILED	    	| Command execution failed on Saturn reader			 							|
			| OTI_UNSUPPORTED		| Command not supported by Saturn reader										|

* See MapErrorToStatus() for extra error messages returned by ExchangeCmd()
*
* \ingroup UI
********************************************************************************************************************/
OTI_STATUS SetOutPortControl(OTI_OUTPORT outport, //!< [in] Outport Mode (PCD, Host or Host-LCD)
	PMSG_BUFF msgBuf) //!< [in] Buffer for building message
{
    RESP_MSG_STRUCT resp;
    OTI_STATUS ret;
    BYTE cmd[] = {0xDF, 0x0A, 0x01, 0x00};

    WORD msgBufLen = msgBuf->Max_Length;

    cmd[3] = (BYTE)outport;

    ret = ExchangeCmd(OP_SET, cmd, sizeof(cmd), msgBuf->Data, &msgBufLen, &resp, SAT_DEFAULT_TIMEOUT);

    if(ret != OTI_OK)
    {
        LOG_DEBUG("SET OUTPORT CTL CMD ERROR");
        return ret;
    }
    msgBuf->Actual_Length = msgBufLen;
    if(resp.succ == NULL)
    {
        if(resp.fail != NULL)
        {
            if(resp.failLen == 4)   // DF 0A 01 ERR
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

	return ret;
}// SetOutPortControl()

//end
