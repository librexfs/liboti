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
#include "platform_utils.h"

/* Doxygen format:*/
/************************** SetReaderPort() ************************************************************************//**
* Sets the port for Saturn reader.
*
* This API Initialise Saturn port and stores the port in config file.

* @return	 OTI_STATUS				\n
			| Value					|	Description																	|
			|-----------------------|-------------------------------------------------------------------------------|
			| OTI_OK				| Function completed successfully				 								|
			| OTI_INVALID_PARAM		| Provided parameter is invalid													|
			| OTI_ERROR			 	| Error						 													|

* See MapErrorToStatus() for extra error messages returned by ExchangeCmd()
* todo does it actually store port number in config file?
*
* \ingroup CFG
********************************************************************************************************************/
OTI_STATUS SetReaderPort(BYTE *port) //!< [in] Port number
{
    int portLen;

    if(!port)
        return OTI_INVALID_PARAM;
        
    portLen = strlen((char *)port);
    if(portLen >= sizeof(gConf.port))
        return OTI_INVALID_PARAM;
        
	if(GetConfig())
	{
        FormatDevicePath(gConf.port, port);

		if(InitPort() == OTI_OK)
			return OTI_OK;
	}
	return OTI_ERROR;

}// SetReaderPort()

/* Doxygen format:*/
/************************** SetReaderPortSpeed() ******************************************************************//**
* Sets communication port speed (only for the terminal).
*
* This API Initialise Saturn port and stores the port in config file. \n
* To set the reader baud rate, use SetSpeed() API.

* @return	 OTI_STATUS				\n
			| Value					|	Description																	|
			|-----------------------|-------------------------------------------------------------------------------|
			| OTI_OK				| Function completed successfully				 								|
			| OTI_CONFIG_ERROR		| Writing config to file failed													|

* See MapErrorToStatus() for extra error messages returned by ExchangeCmd()
* todo does it actually store port number in config file?
*
* \ingroup CFG
********************************************************************************************************************/
OTI_STATUS SetReaderPortSpeed(BAUD_TYPE rate) //!< [in] Baudrate to use when sending data to Saturn
{
	if(GetConfig())
	{
        gConf.baud_rate = rate;
		if(InitPort() == OTI_OK)
			return OTI_OK;
	}
    return OTI_CONFIG_ERROR;
}// SetReaderPortSpeed()

// end
