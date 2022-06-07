#ifndef _SAT_ASSERT_H_
#define _SAT_ASSERT_H_

#include "oti_types.h"

OTI_STATUS SAT_Error(OTI_STATUS status);
void SAT_Assert_Log_Append_Error(const char* str_Condition_Description,
	const char* str_Function_Name, int n_Line_Number);
void SAT_Assert_Log_Append_Check_Failure(const char* str_Function_Call,
	const char* str_Function_Name, int n_Line_Number);


//! \internal
//! \def 	SAT_ASSERT_STATUS(_condition_,error_status)
//!				returns from the function if a condition is false, printing the condition to the host and an internal log.
#define SAT_ASSERT_STATUS(_condition_,error_status)                                  \
{                                                                                   \
  if (!(_condition_))                                                               \
    	{                                                                                 \
    SAT_Assert_Log_Append_Error (#_condition_,__FUNCTION__,__LINE__);       					\
    return SAT_Error(error_status);                                  								\
    	}                                                                                 \
}


//! \def 			SAT_ASSERT(_condition_)
//!	\brief		Returns from the current function with a failure status if the condition is false.
//!	\details	The condition may be any code that returns a boolean value.
//!						If \ref _condition_ is true - then execution continues
#define SAT_ASSERT(_condition_) SAT_ASSERT_STATUS(_condition_, OTI_INVALID_PARAM)  

//! \def 		SAT_ASSERT_FAIL(fail_reason)
//!	\brief	Returns from the current function with a failure status
#define SAT_ASSERT_FAIL(fail_reason) SAT_ASSERT_STATUS(false, fail_reason)

//! \def 		SAT_ASSERT_BOOL(b)
//!	\brief	returns from the current function with a failure status if a bool is not true (1) of false (0)
#define SAT_ASSERT_BOOL(b) SAT_ASSERT_STATUS((b) <= 1, OTI_INVALID_PARAM)

//! \def 			SAT_CHECK_INIT
//!	\brief		must be called at the top of each function that calls CHECK macro. 
//!	\details 	assigns a local variable to save the check result.
#define SAT_CHECK_INIT OTI_STATUS _status_;

//! \def 		SAT_CHECK(_op_)
//!	\brief	checks if an operation fails. if it does - return from the function
#define SAT_CHECK(_op_)                                          										\
{                                                            												\
  _status_ = (_op_);                                         												\
  if (_status_ != OTI_OK)                              											\
      {                                                          												\
    SAT_Assert_Log_Append_Check_Failure (#_op_,__FUNCTION__,__LINE__);      \
    return _status_;                                         												\
      }                                                          												\
}

//! \def 		SAT_CHECK_SILENT(_op_)
//!	\brief	checks if an operation fails. if it does - return from the function, but doesn't log.
#define SAT_CHECK_SILENT(_op_)                                          							\
{                                                            												\
  _status_ = (_op_);                                         												\
  if (_status_ != OTI_OK)                              											\
  {                                                          												\
	return _status_;                                         												\
  }                                                          												\
}

#endif // _SAT_ASSERT_H_

/**
* @}
*/

