//--------------------------------------------------------------------------------------------------
// Implements
//--------------------------------------------------------------------------------------------------
#include "SAT_Assert.h"

#include "otilib.h"
//--------------------------------------------------------------------------------------------------
// Functions
//--------------------------------------------------------------------------------------------------

/************************** oT_Assert_Log_Append_Error *************************************//**
																							 * @internal
																							 * @brief 	logs details of an assert failure
																							 ******************************************************************************/
void SAT_Assert_Log_Append_Error(
	const char* str_Condition_Description, 	//!<	[in] a string descibing the positive case of the condition
	const char* str_Function_Name,          //!<	[in] name of calling function
	int n_Line_Number												//!<  [in] line number in source file
	)
{
	LOG_OUTPUT("*** Error on %s in function %s(), line %d",str_Condition_Description, str_Function_Name, n_Line_Number);
}

/************************** oT_Assert_Log_Append_Check_Failure *************************************//**
* @internal
* @brief 	logs details of a check failure which is usually
******************************************************************************/
void SAT_Assert_Log_Append_Check_Failure(
	const char* str_Function_Call, 					//!<	[in] a string descibing the function call that failed
	const char* str_Function_Name,          //!<	[in] name of calling function
	int n_Line_Number												//!<  [in] line number in source file
	)
{
	LOG_DEBUG("| on %s in function %s(), line %d", str_Function_Call, str_Function_Name, n_Line_Number);
}



/*************************************************************************//**
* @brief 		Returns the received error code and turns on all LEDs.
* @details	When the application code is returning and error - it's a good practice to wrap it with this function
*						to enable a breakpoint
*						Called by CHECK and ASSERT macros when an error occurs.
*						Put a breakpoint here to catch any error.
* @retval		The status code it received
******************************************************************************/
OTI_STATUS SAT_Error(OTI_STATUS status)
{
	return status;
}

//---------------------------------------------------------------------------------------------------------
