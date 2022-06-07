#ifndef _ACTIVATETRANSACTIONPARSER_H_
#define _ACTIVATETRANSACTIONPARSER_H_

#include "protocol.h"

#include "oti_types.h"

#ifdef  __cplusplus
extern "C" {
#endif

BOOLEAN TLV_StepIn(TLV_MSG *buffer, int tag);
BOOLEAN TLV_ReadTag(TLV_MSG *buffer, int *tag, TLV_MSG *data);
void ResetPosition(TLV_MSG *buffer);
BOOLEAN TLV_SearchTag(TLV_MSG *buffer, int tlv_tag, int tagToFind, TLV_MSG *data);

#ifdef  __cplusplus
}
#endif

#endif
