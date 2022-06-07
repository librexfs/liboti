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
#include "protocol.h"
#include "frames.h"
#include "otilib.h"
#include "config.h"

/* Doxygen format:*/
/************************** GetSaturnConfig() *********************************************************************//**
* Read Saturn reader configuration stored in config file.
*
*
* @return	 OTI_STATUS				\n
			| Value					|	Description																	|
			|-----------------------|-------------------------------------------------------------------------------|
			| OTI_OK				| Config read successfully.						 								|
			| OTI_CONFIG_ERROR    	| Reading config from file failed					 							|

*
* \ingroup CFG
********************************************************************************************************/
OTI_STATUS GetSaturnConfig(OTI_CONFIG_STRUCT *config) /**< [out]  Pointer to an OTI_CONFIG structure to fill. 
															See OTI_CONFIG_STRUCT definition in otilib.h */
{
	if(GetConfig())
	{
		memcpy(config, &gConf, sizeof(OTI_CONFIG_STRUCT));
		return OTI_OK;
	}
    return OTI_CONFIG_ERROR;
} // GetSaturnConfig()
// end
