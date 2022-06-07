#include "oti_types.h"


// Get string describing oti status code
const char* OtiStatusStr(OTI_STATUS code)
{
    switch(code) {
    case OTI_ERROR:                       return "OTI_ERROR";
    case OTI_OK:                          return "OTI_OK";
    case OTI_UNSUPPORTED:                 return "OTI_UNSUPPORTED";
    case OTI_FAILED:                      return "OTI_FAILED";
    case OTI_NAK:                         return "OTI_NAK";
    case OTI_DATA_MISSING:                return "OTI_DATA_MISSING";
    case OTI_LRC_ERROR:                   return "OTI_LRC_ERROR";
    case OTI_TLV_ERROR:                   return "OTI_TLV_ERROR";
    case OTI_INVALID_PARAM:               return "OTI_INVALID_PARAM";
    case OTI_STX_MISSING:                 return "OTI_STX_MISSING";
    case OTI_ETX_MISSING:                 return "OTI_ETX_MISSING";
    case OTI_UNKNOWN_OPCODE:              return "OTI_UNKNOWN_OPCODE";
    case OTI_TOO_MANY_INSTRUCTIONS:       return "OTI_TOO_MANY_INSTRUCTIONS";
    case OTI_PARAM_NOT_SUPPORTED:         return "OTI_PARAM_NOT_SUPPORTED";
    case OTI_MEMORY_ERROR:                return "OTI_MEMORY_ERROR";
    case OTI_BUFFER_OVERFLOW:             return "OTI_BUFFER_OVERFLOW";
    case OTI_COM_ERROR:                   return "OTI_COM_ERROR";
    case OTI_COM_TIMEOUT:                 return "OTI_COM_TIMEOUT";
    case OTI_CONFIG_ERROR:                return "OTI_CONFIG_ERROR";
    case OTI_INVALID_DATA:                return "OTI_INVALID_DATA";
    case OTI_INCOMPLETE_DATA:             return "OTI_INCOMPLETE_DATA";
    case OTI_UNKNOWN_DATA:                return "OTI_UNKNOWN_DATA";
    case OTI_COLLISION_ERROR:             return "OTI_COLLISION_ERROR";
    case OTI_PCD_MEM_WRITE_ERROR:         return "OTI_PCD_MEM_WRITE_ERROR";
    case OTI_PCD_MEM_READ_ERROR:          return "OTI_PCD_MEM_READ_ERROR";
    case OTI_RF_FAILURE:                  return "OTI_RF_FAILURE";
    case OTI_RF_FAILURE_AFTER_RECOVERY:   return "OTI_RF_FAILURE_AFTER_RECOVERY";
    case OTI_DATA_LENGTH_OVERFLOW:        return "OTI_DATA_LENGTH_OVERFLOW";
    case OTI_HERCULES_ERROR_CODE:         return "OTI_HERCULES_ERROR_CODE";
    case OTI_SAM_CONTACT_FAILURE:         return "OTI_SAM_CONTACT_FAILURE";
    case OTI_SAM_MISSING:                 return "OTI_SAM_MISSING";
    case OTI_SAM_NOT_ACTIVE:              return "OTI_SAM_NOT_ACTIVE";
    case OTI_SAM_PROTOCOL_ERROR:          return "OTI_SAM_PROTOCOL_ERROR";
    case OTI_MISSING_PARAM:               return "OTI_MISSING_PARAM";
    case OTI_APPLI_DISABLED:              return "OTI_APPLI_DISABLED";
    case OTI_SUB_SCHEME_DISABLED:         return "OTI_SUB_SCHEME_DISABLED";
    case OTI_INDEX_NOT_FOUND:             return "OTI_INDEX_NOT_FOUND";
    case OTI_KEY_HASH_MISMATCH:           return "OTI_KEY_HASH_MISMATCH";
    case OTI_AID_LENGTH_ERROR:            return "OTI_AID_LENGTH_ERROR";
    case OTI_DATA_LENGTH_ERROR:           return "OTI_DATA_LENGTH_ERROR";
    case OTI_PCD_SETUP_ERROR:             return "OTI_PCD_SETUP_ERROR";
    case OTI_SUBSCHEME_NOT_APPLICABLE:    return "OTI_SUBSCHEME_NOT_APPLICABLE";
    case OTI_PKI_STORAGE_ERROR:           return "OTI_PKI_STORAGE_ERROR";
    case OTI_EMV_TRANSACTION_TERMINATED:  return "OTI_EMV_TRANSACTION_TERMINATED";
    case OTI_POLL_TIMEOUT:                return "OTI_POLL_TIMEOUT";
    case OTI_EMV_TRANSACTION_CANCELLED:   return "OTI_EMV_TRANSACTION_CANCELLED";
    case OTI_READER_BUSY:                 return "OTI_READER_BUSY";
    case OTI_BOOT_CRC_ERROR:              return "OTI_BOOT_CRC_ERROR";
    case OTI_BOOT_PACKET_SEQ_ERROR:       return "OTI_BOOT_PACKET_SEQ_ERROR";
    case OTI_BOOT_TARGET_ID_ERROR:        return "OTI_BOOT_TARGET_ID_ERROR";
    case OTI_BOOT_PACKET_LEN_ERROR:       return "OTI_BOOT_PACKET_LEN_ERROR";
    case OTI_BOOT_FILE_TOO_BIG:           return "OTI_BOOT_FILE_TOO_BIG";
    case OTI_BOOT_STAMP_ERROR:            return "OTI_BOOT_STAMP_ERROR";
    case OTI_BOOL_FILE_TARGET_ERROR:      return "OTI_BOOL_FILE_TARGET_ERROR";
    case OTI_BOOT_KEY_VERSION_ERROR:      return "OTI_BOOT_KEY_VERSION_ERROR";
    case OTI_BOOT_HASH_ALGO_ERROR:        return "OTI_BOOT_HASH_ALGO_ERROR";
    case OTI_BOOT_META_NOT_ORIGINAL:      return "OTI_BOOT_META_NOT_ORIGINAL";
    case OTI_BOOT_META_MISS_MANDATORY:    return "OTI_BOOT_META_MISS_MANDATORY";
    case OTI_BOOT_META_TLV_ERROR:         return "OTI_BOOT_META_TLV_ERROR";
    case OTI_BOOT_META_LEN_ERROR:         return "OTI_BOOT_META_LEN_ERROR";
    case OTI_BOOT_META_VERSION_ERROR:     return "OTI_BOOT_META_VERSION_ERROR";
    case OTI_PARAM_MISSING:               return "OTI_PARAM_MISSING";
    case OTI_INSUFFICIENT_CREDIT:         return "OTI_INSUFFICIENT_CREDIT";
    case OTI_BUFFER_LENGTH:               return "OTI_BUFFER_LENGTH";
    default:                              return "Unknown code";
    }
}
