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
#include "otilib.h"
#include "config.h"

/* Doxygen format:*/
/************************** SetDebug() ****************************************************************************//**
* Sets debug on/off.
*
* @return	 OTI_STATUS				\n
			| Value					|	Description																	|
			|-----------------------|-------------------------------------------------------------------------------|
			| OTI_OK				| Function completed successfully				 								|
			| OTI_INVALID_PARAM		| Boolean value invalid															|
			| OTI_CONFIG_ERROR		| Writing config to file failed													|

* See MapErrorToStatus() for extra error messages returned by ExchangeCmd()
*
* \ingroup CFG
********************************************************************************************************************/
OTI_STATUS SetDebug(BOOLEAN debug) //!< [in] Boolean value (TRUE or ON for on, FALSE or OFF for off)
{
	if((debug != TRUE) && (debug!=FALSE))
        return OTI_INVALID_PARAM;
	
	if(GetConfig())
	{
		gConf.debug = debug;
		return OTI_OK;
	}
    return OTI_CONFIG_ERROR;
}// setdebug()
// end
