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
#include "otilib.h"
#include "config.h"
#include "serial.h"

/* Doxygen format:*/
/************************** OTIInit() ****************************************************************************//**
* Sets defaults (initial port, baudrate and debug mode).
*
* @return	 OTI_STATUS				\n
			| Value					|	Description																	|
			|-----------------------|-------------------------------------------------------------------------------|
			| OTI_OK				| Function completed successfully				 								|
			| OTI_INVALID_PARAM		| Provided parameter is invalid													|
			| OTI_CONFIG_ERROR    	| Writing config to file failed						 							|

* See MapErrorToStatus() for extra error messages returned by ExchangeCmd()
*
* \ingroup GENERAL
********************************************************************************************************************/
OTI_STATUS OTIInit(	BYTE *port,				//!< [in] Port to which the Saturn reader is connected
					BAUD_TYPE baud_rate,	//!< [in] Baudrate to communicate with the Saturn reader
					STOP_TYPE stop_bits,	//!< [in] Stop bits to communicate with the Saturn reader
					BOOLEAN debug)			//!< [in] Boolean value (TRUE or ON for on, FALSE or OFF for off)
{
    WORD portLen;
    
	if((debug != TRUE) && (debug != FALSE))
        return OTI_INVALID_PARAM;

    if(!port)
        return OTI_INVALID_PARAM;
        
    portLen = strlen((char *)port);
    if(portLen >= sizeof(gConf.port))
        return OTI_INVALID_PARAM;
        
    if((baud_rate < BPS_50) || (baud_rate >= BPS_LAST))
        return OTI_INVALID_PARAM;
        
    if((stop_bits != ONE_STOPBIT) && (stop_bits != TWO_STOPBITS))
        return OTI_INVALID_PARAM;

	if(SetDefaultConfig())
	{
        memcpy(gConf.port, port, portLen);
        gConf.port[portLen] = 0x00;
	    gConf.debug = debug;
        gConf.baud_rate = baud_rate;
        gConf.stop_bits = stop_bits;
	    if(InitPort() == OTI_OK)
		    return OTI_OK;
	}
    return OTI_CONFIG_ERROR;
}// OTIInit()
// end
