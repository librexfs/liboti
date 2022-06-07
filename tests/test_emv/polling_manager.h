#ifndef _POLLING_MANAGER_H_
#define _POLLING_MANAGER_H_

#include "otilib.h"
#include "protocol.h"

#define MAX_BUFFER_SIZE 1024

// moved from SAT_Assert.h to avoid warnings in main project
static UINT __retries = 0;
#define SAT_RETRY(sat_operation)                               \
{                                                              \
    __retries = 5;                                               \
        while(((sat_operation) != OTI_OK) && (--__retries > 0)) {    \
        SystemSleep(5);                                             \
            }                                                            \
}

#include <stdbool.h>

typedef BOOLEAN(*OnOnlineRequest)(TLV_MSG *fc, TLV_MSG *incomingE8);

typedef void(*OnOfflineRequest)(TLV_MSG *fc);

void InitTransaction(OnOnlineRequest callbackOnline, OnOfflineRequest callbackOffline);

XRESULT DoTransaction(ACT_EMV_STRUCT *act_trans, TLV_MSG *resp);

#endif
