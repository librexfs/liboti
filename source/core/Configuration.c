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
#include <string.h>
#include "config.h"
#include "otilib.h"

// Global variables for Serial configuration
OTI_CONFIG_STRUCT gConf;
BOOLEAN gConfSet = FALSE;


/* Doxygen format:*/
/************************** GetConfig() **************************************************************************//**
* Retrieve OTILIB Communication COM port configuration.
*
* @return	 OTI_STATUS				\n
			| Value					|	Description																	|
			|-----------------------|-------------------------------------------------------------------------------|
			| TRUE					| Function completed successfully				 								|
			| FALSE					| File error occured															|

*
* \ingroup INTERNAL
********************************************************************************************************************/
BOOLEAN GetConfig(void)
{
    // Don't bother with config if unnecessary
    if(gConfSet)
        return TRUE;

    SetDefaultConfig();

    gConfSet = TRUE;

    return TRUE ;
} // GetConfig()


/* Doxygen format:*/
/************************** SetDefaultConfig() *********************************************************************//**
* Write communicaton config with default settings.
*
* @return	 OTI_STATUS				\n
			| Value					|	Description																	|
			|-----------------------|-------------------------------------------------------------------------------|
			| TRUE					| Function completed successfully				 								|
			| FALSE					| File error occured															|

*
* \ingroup INTERNAL
********************************************************************************************************************/
BOOLEAN SetDefaultConfig()
{
    // This is default OTILIB configuration
    memcpy(gConf.port, "/dev/ttyACM0\x00", 13);
    gConf.baud_rate = BPS_115200;
    gConf.stop_bits = TWO_STOPBITS;
    gConf.poll_timeout = POLL_TIMEOUT_DEFAULT;
    gConf.debug = TRUE;

    return TRUE;
} // SetDefaultConfig()
// end
