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
#include <stdlib.h>
#include "otilib.h"
#include "oti_types.h"
#include "protocol.h"
#include "frames.h"
#include "tlv.h"
#include "config.h"

/* Doxygen format:*/
/************************** ParseEMVResp() *************************************************************************//**
* Extracts Clearing Record Parameters from Response to ActivateTransaction() command.
*
* This function should be extended with new data objects (tags) when additional data objects to retrieve in the
* transaction result are added to the list, using the "MagStripe Result Tag List" comamnd (Tag DF 66) or the
* "EMV Result Tag List" (Tag DF 67). See more details in the "oti Host-PCD Communication Protocol" documentation.
*
* @return	 OTI_STATUS				\n
| Value					|	Description																	|
|-----------------------|-------------------------------------------------------------------------------|
| OTI_OK				| Function completed successfully				 								|
| OTI_TLV_ERROR			| Error while parsing response data TLVs										|
| OTI_INVALID_DATA		| Response data is not included in a 0xFC TLV template							|
| OTI_DATA_LENGTH_ERROR | Retrieved data doesn't fit inside its appropriate response structure member	|

* See MapErrorToStatus() for extra error messages returned by ExchangeCmd()
*
* \ingroup INTERNAL
********************************************************************************************************************/
OTI_STATUS ParseEMVResp(PMSG_BUFF msgBuf,//!< [in] Buffer for building message
	EMV_RESP_STRUCT *resp)	//!< [out] Pointer to an EMV Response structure to fill
{
	static OTI_TLV curTag;
	BYTE *dataPtr;
	BYTE *endPtr;
	WORD respLen;
	WORD tagLen;

	dataPtr = msgBuf->Data;
	endPtr = dataPtr + msgBuf->Max_Length;

	/* Parse every TLV in the response tag FC. */
	if (*dataPtr != 0xFC)
	{
		LOG_DEBUG("WRONG TEMPLATE IN RESP");
		return OTI_INVALID_DATA;
	}
	dataPtr++;
	respLen = ParseLen(&dataPtr, endPtr, NULL);
	if (respLen == TLV_LEN_ERROR)
		return OTI_TLV_ERROR;

	/* Now dataPtr points on the list of EMV result TLVs */
	endPtr = dataPtr + respLen;
	while (dataPtr < endPtr)
	{
		if (BuildTlvFromRaw(dataPtr, respLen, dataPtr, respLen,&curTag))
			return OTI_TLV_ERROR;

		tagLen = curTag.len;
		/* Assign object value to response structure member */
		switch (curTag.tag)
		{
		case 0xDF23:    // Track1
			if (tagLen > sizeof(resp->track1_data))
			{
				return OTI_DATA_LENGTH_ERROR;
			}

			resp->track1_len = (BYTE)tagLen;
			memcpy(resp->track1_data, curTag.value, tagLen);
			resp->properties1 |= RESP1_TRACK1;
			break;

		case 0xDF22:    // Track2
			if (tagLen > sizeof(resp->track2_data))
			{
				return OTI_DATA_LENGTH_ERROR;
			}
			resp->track2_len = (BYTE)tagLen;
			memcpy(resp->track2_data, curTag.value, tagLen);
			resp->properties1 |= RESP1_TRACK2;
			break;

		case 0x8E:      // CVM List
			if (tagLen > sizeof(resp->cvm_list))
			{
				return OTI_DATA_LENGTH_ERROR;
			}
			resp->cvm_list_len = tagLen;
			memcpy(resp->cvm_list, curTag.value, tagLen);
			resp->properties1 |= RESP1_CVM_LIST;
			break;

		case 0x9F34:    // CVM Results
			if (tagLen != sizeof(resp->cvm_results))
			{
				return OTI_DATA_LENGTH_ERROR;
			}
			memcpy(resp->cvm_results, curTag.value, tagLen);
			resp->properties1 |= RESP1_CVM_RESULTS;
			break;

		case 0x9F37:    // Unpredictable Number
			if (tagLen != sizeof(resp->unpred_num))
			{
				return OTI_DATA_LENGTH_ERROR;
			}
			memcpy(resp->unpred_num, curTag.value, tagLen);
			resp->properties1 |= RESP1_UNPRED_NUM;
			break;

		case 0x95:    // Terminal Verification Result
			if (tagLen != sizeof(resp->tvr))
			{
				return OTI_DATA_LENGTH_ERROR;
			}
			memcpy(resp->tvr, curTag.value, tagLen);
			resp->properties1 |= RESP1_TVR;
			break;

		case 0x9A:    // Transaction Date
			if (tagLen != sizeof(resp->date))
			{
				return OTI_DATA_LENGTH_ERROR;
			}
			memcpy(resp->date, curTag.value, tagLen);
			resp->properties1 |= RESP1_TRANS_DATE;
			break;

		case 0x9F27:    // Cryptogram Information Data
			if (tagLen != sizeof(resp->cid))
			{
				return OTI_DATA_LENGTH_ERROR;
			}
			resp->cid = curTag.value[0];
			resp->properties1 |= RESP1_CID;
			break;

		case 0x9B:    // Transaction Status Information
			if (tagLen != sizeof(resp->tsi))
			{
				return OTI_DATA_LENGTH_ERROR;
			}
			memcpy(resp->tsi, curTag.value, tagLen);
			resp->properties1 |= RESP1_TSI;
			break;

		case 0x9F22:    // CAPK Index
			if (tagLen != sizeof(resp->capk_index))
			{
				return OTI_DATA_LENGTH_ERROR;
			}
			resp->capk_index = curTag.value[0];
			resp->properties1 |= RESP1_CAPK_INDEX;
			break;

		case 0x9F26:    // Application Cryptogram
			if (tagLen != sizeof(resp->app_crypt))
			{
				return OTI_DATA_LENGTH_ERROR;
			}
			memcpy(resp->app_crypt, curTag.value, tagLen);
			resp->properties1 |= RESP1_APP_CRYPT;
			break;

		case 0x9F10:    // Issuer Application Data
			if (tagLen > sizeof(resp->iss_app_data))
			{
				return OTI_DATA_LENGTH_ERROR;
			}
			resp->iss_app_data_len = (BYTE)tagLen;
			memcpy(resp->iss_app_data, curTag.value, tagLen);
			resp->properties1 |= RESP1_ISS_APPDATA;
			break;

		case 0x84:    // DF Name
			if (tagLen > sizeof(resp->df_name))
			{
				return OTI_DATA_LENGTH_ERROR;
			}
			resp->df_name_len = (BYTE)tagLen;
			memcpy(resp->df_name, curTag.value, tagLen);
			resp->properties1 |= RESP1_DF_NAME;
			break;

		case 0x9F36:    // Application Transaction Counter
			if (tagLen > sizeof(resp->atc))
			{
				return OTI_DATA_LENGTH_ERROR;
			}
			memcpy(resp->atc, curTag.value, tagLen);
			resp->properties1 |= RESP1_ATC;
			break;

		case 0xDF69:    // Reader Transaction Result
			if (tagLen > sizeof(resp->pcd_trans_result))
			{
				return OTI_DATA_LENGTH_ERROR;
			}
			resp->pcd_trans_result = curTag.value[0];
			resp->properties1 |= RESP1_TRX_RESULT;
			break;

		case 0x9F45:    // Data Authentication Code
			if (tagLen > sizeof(resp->data_auth_code))
			{
				return OTI_DATA_LENGTH_ERROR;
			}
			memcpy(resp->data_auth_code, curTag.value, tagLen);
			resp->properties1 |= RESP1_DATA_AUTH_CODE;
			break;

		case 0x82:    // Application Interchange Profile
			if (tagLen > sizeof(resp->aip))
			{
				return OTI_DATA_LENGTH_ERROR;
			}
			memcpy(resp->aip, curTag.value, tagLen);
			resp->properties1 |= RESP1_AIP;
			break;

		case 0x9F66:    // Visa TTQ
			if (tagLen > sizeof(resp->ttq))
			{
				return OTI_DATA_LENGTH_ERROR;
			}
			memcpy(resp->ttq, curTag.value, tagLen);
			resp->properties1 |= RESP1_TTQ;
			break;

		case 0x5A:    // Application PAN
			if (tagLen > sizeof(resp->pan))
			{
				return OTI_DATA_LENGTH_ERROR;
			}
			memcpy(resp->pan, curTag.value, tagLen);
			resp->pan_len = (BYTE)tagLen;
			resp->properties1 |= RESP1_PAN;
			break;

		case 0x5F34:    // Application PAN Sequence Number
			if (tagLen > sizeof(resp->pan_seq_num))
			{
				return OTI_DATA_LENGTH_ERROR;
			}
			resp->pan_seq_num = *(curTag.value);
			resp->properties1 |= RESP1_PAN_SEQ;
			break;

		case 0x5F24:    // Application Expiry Date
			if (tagLen > sizeof(resp->exp_date))
			{
				return OTI_DATA_LENGTH_ERROR;
			}
			memcpy(resp->exp_date, curTag.value, tagLen);
			resp->properties1 |= RESP1_EXP_DATE;
			break;

		case 0x50:    // Application Label
			if (tagLen > sizeof(resp->app_label))
			{
				return OTI_DATA_LENGTH_ERROR;
			}
			memcpy(resp->app_label, curTag.value, tagLen);
			resp->app_label_len = (BYTE)tagLen;
			resp->properties1 |= RESP1_APP_LABEL;
			break;

		case 0x9F4C:    // ICC Dynamic Number
			if (tagLen > sizeof(resp->icc_dyn_num))
			{
				return OTI_DATA_LENGTH_ERROR;
			}
			memcpy(resp->icc_dyn_num, curTag.value, tagLen);
			resp->icc_dyn_num_len = (BYTE)tagLen;
			resp->properties1 |= RESP1_ICC_DYN_NUM;
			break;

		case 0x5F20:    // Cardholder Name
			if (tagLen > sizeof(resp->cardholder_name))
			{
				return OTI_DATA_LENGTH_ERROR;
			}
			memcpy(resp->cardholder_name, curTag.value, tagLen);
			resp->cardholder_name_len = (BYTE)tagLen;
			resp->properties1 |= RESP1_HOLDER_NAME;
			break;

		case 0x9F07:    // Application Usage Control
			if (tagLen > sizeof(resp->auc))
			{
				return OTI_DATA_LENGTH_ERROR;
			}
			memcpy(resp->auc, curTag.value, tagLen);
			resp->properties1 |= RESP1_AUC;
			break;

		case 0x9F0D:    // Issuer Action Code - Default
			if (tagLen > sizeof(resp->iac_default))
			{
				return OTI_DATA_LENGTH_ERROR;
			}
			memcpy(resp->iac_default, curTag.value, tagLen);
			resp->properties1 |= RESP1_IAC;
			break;

		case 0x9F0E:    // Issuer Action Code - Denial
			if (tagLen > sizeof(resp->iac_denial))
			{
				return OTI_DATA_LENGTH_ERROR;
			}
			memcpy(resp->iac_denial, curTag.value, tagLen);
			resp->properties1 |= RESP1_IAC;
			break;

		case 0x9F0F:    // Issuer Action Code - Online
			if (tagLen > sizeof(resp->iac_online))
			{
				return OTI_DATA_LENGTH_ERROR;
			}
			memcpy(resp->iac_online, curTag.value, tagLen);
			resp->properties1 |= RESP1_IAC;
			break;

		case 0x89:    // Authorisation Code
			if (tagLen > sizeof(resp->auth_code))
			{
				return OTI_DATA_LENGTH_ERROR;
			}
			memcpy(resp->auth_code, curTag.value, tagLen);
			resp->properties1 |= RESP1_AUTH_CODE;
			break;

		case 0x9F5D:    // Available Offline Spending Amount
			if (tagLen > sizeof(resp->offline_spending_amount))
			{
				return OTI_DATA_LENGTH_ERROR;
			}
			memcpy(resp->offline_spending_amount, curTag.value, tagLen);
			resp->properties1 |= RESP1_OFF_AMOUNT;
			break;

		case 0x4F:    // AID
			if (tagLen > sizeof(resp->aid))
			{
				return OTI_DATA_LENGTH_ERROR;
			}
			resp->aid_len = (BYTE)tagLen;
			memcpy(resp->aid, curTag.value, tagLen);
			resp->properties1 |= RESP1_AID;
			break;

		case 0xDF16:    // Detected PICC Type
			if (tagLen != sizeof(resp->picc_type))
			{
				return OTI_DATA_LENGTH_ERROR;
			}
			resp->picc_type = curTag.value[0];
			resp->properties1 |= RESP1_PICC_TYPE;
			break;

		case 0xDF68:    // Poll EMV Error
			if (tagLen != sizeof(resp->poll_emv_error))
			{
				return OTI_DATA_LENGTH_ERROR;
			}
			resp->poll_emv_error = curTag.value[0];
			resp->properties1 |= RESP1_POLL_EMV_ERROR;
			break;

		case 0x9F02:    // Amount Authorized
			if (tagLen != sizeof(resp->amount_authorized))
			{
				return OTI_DATA_LENGTH_ERROR;
			}
			memcpy(resp->amount_authorized, curTag.value, tagLen);
			resp->properties1 |= RESP1_AMOUNT_AUTHORIZED;
			break;

		case 0x9C:    // Transaction Type
			if (tagLen != sizeof(resp->trans_type))
			{
				return OTI_DATA_LENGTH_ERROR;
			}
			resp->trans_type = curTag.value[0];
			resp->properties2 |= RESP2_TRANS_TYPE;
			break;

		case 0xDF70:    // Data Channel
			if (tagLen != sizeof(resp->data_channel))
			{
				return OTI_DATA_LENGTH_ERROR;
			}
			resp->data_channel = curTag.value[0];
			resp->properties2 |= RESP2_DATA_CHANNEL;
			break;

		case 0x5F54:    // Bank Identified Code
			if (tagLen > sizeof(resp->bank_id_code))
			{
				return OTI_DATA_LENGTH_ERROR;
			}
			resp->bank_id_code_len = (BYTE)tagLen;
			memcpy(resp->bank_id_code, curTag.value, tagLen);
			resp->properties2 |= RESP2_BANK_ID_CODE;
			break;

		case 0x9F05:    // Application Discretionary Data
			if (tagLen > sizeof(resp->app_disc_data))
			{
				return OTI_DATA_LENGTH_ERROR;
			}
			resp->app_disc_data_len = (BYTE)tagLen;
			memcpy(resp->app_disc_data, curTag.value, tagLen);
			resp->properties2 |= RESP2_APP_DISC_DATA;
			break;

		case 0x9F41:    // Transaction Sequence Counter
			if (tagLen > sizeof(resp->trans_seq_counter))
			{
				return OTI_DATA_LENGTH_ERROR;
			}
			resp->trans_seq_counter_len = (BYTE)tagLen;
			memcpy(resp->trans_seq_counter, curTag.value, tagLen);
			resp->properties2 |= RESP2_TRANS_SEQ_CNTR;
			break;

		case 0x9F0B:    // Cardholder Name Extended
			if (tagLen > sizeof(resp->holder_name_ext))
			{
				return OTI_DATA_LENGTH_ERROR;
			}
			resp->holder_name_ext_len = (BYTE)tagLen;
			memcpy(resp->holder_name_ext, curTag.value, tagLen);
			resp->properties2 |= RESP2_HOLDER_NAME_EXT;
			break;

		case 0x9F6E:    // Form Factor Indicator (VISA) / PayPass Third Party Data (MasterCard)
			if (tagLen != sizeof(resp->form_factor_ind))
			{
				if (tagLen > sizeof(resp->paypass_3rd_party_data))
				{
					return OTI_DATA_LENGTH_ERROR;
				}
				resp->paypass_3rd_party_data_len = (BYTE)tagLen;
				memcpy(resp->paypass_3rd_party_data, curTag.value, tagLen);
				resp->properties2 |= RESP2_PP_3RD_PARTY_DATA;
			}
			else
			{
				memcpy(resp->form_factor_ind, curTag.value, tagLen);
				resp->properties2 |= RESP2_FORM_FACTOR_IND;
			}
			break;

		case 0x9F7C:    // Customer Exclusive Data
			if (tagLen > sizeof(resp->customer_exclusive_data))
			{
				return OTI_DATA_LENGTH_ERROR;
			}
			resp->customer_exclusive_data_len = (BYTE)tagLen;
			memcpy(resp->customer_exclusive_data, curTag.value, tagLen);
			resp->properties2 |= RESP2_CUST_EXCL_DATA;
			break;

		case 0x5F25:    // Application Effective Date
			if (tagLen != sizeof(resp->app_effective_date))
			{
				return OTI_DATA_LENGTH_ERROR;
			}
			memcpy(resp->app_effective_date, curTag.value, tagLen);
			resp->properties2 |= RESP2_APP_EFFECTIVE_DATE;
			break;

		case 0x9F6C:    // Card Transaction Qualifiers
			if (tagLen != sizeof(resp->ctq))
			{
				return OTI_DATA_LENGTH_ERROR;
			}
			memcpy(resp->ctq, curTag.value, tagLen);
			resp->properties2 |= RESP2_CTQ;
			break;

		case 0x9F6D:    // MS Application Version Number
			if (tagLen != sizeof(resp->ms_app_ver_num))
			{
				return OTI_DATA_LENGTH_ERROR;
			}
			memcpy(resp->ms_app_ver_num, curTag.value, tagLen);
			resp->properties2 |= RESP2_MS_APP_VER_NUM;
			break;

		case 0x9F1E:    // PCD Serial Number
			if (tagLen != sizeof(resp->pcd_serial_num))
			{
				return OTI_DATA_LENGTH_ERROR;
			}
			memcpy(resp->pcd_serial_num, curTag.value, tagLen);
			resp->properties2 |= RESP2_PCD_SERIAL_NUM;
			break;

		case 0x9F33:    // Terminal Capabilities
			if (tagLen != sizeof(resp->term_capabilities))
			{
				return OTI_DATA_LENGTH_ERROR;
			}
			memcpy(resp->term_capabilities, curTag.value, tagLen);
			resp->properties2 |= RESP2_TERM_CAPABILITIES;
			break;

		case 0xDF810C:    // Terminal Capabilities - No CVM
			if (tagLen != sizeof(resp->term_capabilities_no_cvm))
			{
				return OTI_DATA_LENGTH_ERROR;
			}
			memcpy(resp->term_capabilities_no_cvm, curTag.value, tagLen);
			resp->properties2 |= RESP2_TERM_CAPABILITIES_NO_CVM;
			break;

		case 0x9F1A:    // Terminal Country Code
			if (tagLen != sizeof(resp->country_code))
			{
				return OTI_DATA_LENGTH_ERROR;
			}
			memcpy(resp->country_code, curTag.value, tagLen);
			resp->properties2 |= RESP2_COUNTRY_CODE;
			break;

		case 0x9F35:    // Terminal Type
			if (tagLen != sizeof(resp->terminal_type))
			{
				return OTI_DATA_LENGTH_ERROR;
			}
			resp->terminal_type = curTag.value[0];
			resp->properties2 |= RESP2_TERMINAL_TYPE;
			break;

		case 0x57:    // Track 2 Equivalent Data
			if (tagLen > sizeof(resp->track2_eq_data))
			{
				return OTI_DATA_LENGTH_ERROR;
			}
			resp->track2_eq_data_len = (BYTE)tagLen;
			memcpy(resp->track2_eq_data, curTag.value, tagLen);
			resp->properties2 |= RESP2_TRACK2_EQ_DATA;
			break;

		case 0x9F53:    // Transaction Category Code
			if (tagLen != sizeof(resp->trans_category_code))
			{
				return OTI_DATA_LENGTH_ERROR;
			}
			resp->trans_category_code = curTag.value[0];
			resp->properties2 |= RESP2_TRANS_CATEGORY_CODE;
			break;

		case 0x5F2A:    // Transaction Currency Code
			if (tagLen != sizeof(resp->trans_currency_code))
			{
				return OTI_DATA_LENGTH_ERROR;
			}
			memcpy(resp->trans_currency_code, curTag.value, tagLen);
			resp->properties2 |= RESP2_TRANS_CURRENCY_CODE;
			break;
		}

		dataPtr += curTag.size;
		respLen -= curTag.size;
	}

	return OTI_OK;
} // ParseEMVResp()


OTI_STATUS GetResponse(BYTE *msgBuf, OTI_STATUS ret, WORD msgLen, WORD respLen, RESP_MSG_STRUCT *resp, TLV_MSG *act_emv_resp, DWORD timeout)
{
	BYTE *bufPtr;
	BYTE *endPtr;
afterResp:
	if (ret != OTI_OK)
	{

		/* Treat special cases if we tried to cancel transaction */

		/* Buffer starting with Cancel ACK */
		if ((msgLen > 8) && (memcmp(msgBuf, "\x02\x08\x00\x3E\x00\x00\x34\x03", 8) == 0))
		{
			/* Case 1: Cancel ACK + Cancel Resp (success) only */
			if ((msgLen == 20) && (memcmp(msgBuf + 8, "\x02\x0C\x00\x3E\xFF\x01\x03\xDF\x7D\x00\x6F\x03", 12) == 0))
			{
				LOG_DEBUG("USER CANCEL\r\n");
				ret = OTI_EMV_TRANSACTION_CANCELLED;
				//printbuf("\xCC\x01\xCC", 3);
			}
			else if (msgLen > 20)
			{
				if (memcmp(msgBuf + 8, "\x02\x0D\x00\x3E\xFF\x03\x04\xDF\x7D\x01\x1E\x74\x03", 13) == 0)
				{
					/* Case 2: Cancel ACK + Cancel Resp (fail) + ActTrans Resp */
					respLen = msgLen - 21;  // msgLen - ACK - Cancel resp
					ret = CheckResp(msgBuf + 21, &respLen, &bufPtr, &endPtr);
					//printbuf("\xCC\x02\xCC", 3);
					//printbuf(msgBuf + 21, respLen);
					if (ret == OTI_OK)
						ret = ParseResp(bufPtr, endPtr, resp);
				}
				else if (memcmp(msgBuf + msgLen - 13, "\x02\x0D\x00\x3E\xFF\x03\x04\xDF\x7D\x01\x1E\x74\x03", 13) == 0)
				{
					/* Case 3: Cancel ACK + ActTrans Resp + Cancel Resp (fail) */
					respLen = msgLen - 21;  // msgLen - ACK - Cancel resp
					ret = CheckResp(msgBuf + 8, &respLen, &bufPtr, &endPtr);
					//printbuf("\xCC\x03\xCC", 3);
					if (ret == OTI_OK)
						ret = ParseResp(bufPtr, endPtr, resp);
				}
			}
		}
		else if (msgLen > 20)   /* Buffer should be starting with Poll Resp */
		{
			/* Case 4: ActTrans Resp + Cancel ACK + Cancel Resp (success) */
			respLen = msgLen - 20;  // msgLen - ACK - Cancel resp (success)
			ret = CheckResp(msgBuf, &respLen, &bufPtr, &endPtr);
			//printbuf("\xCC\x04\xCC", 3);
			if (ret == OTI_OK)
				ret = ParseResp(bufPtr, endPtr, resp);
		}
		/* If after all the checks above we still don't have a clean response
		* there is nothing else we can do.
		*/
		if (ret != OTI_OK)
		{
			LOG_DEBUG("POLLEMV ERROR: %X\r\n", ret);
			return ret;
		}
	}

	/* If we got there, we got some data at least */
	if ((*resp).succ == NULL)
	{
		if ((*resp).fail != NULL)
		{
			memcpy(act_emv_resp->Data, (*resp).fail, (*resp).failLen);
			act_emv_resp->Length = (*resp).failLen;
			act_emv_resp->CurrentLocation = 0;
			if ((*resp).failLen == 4)   // DF 7F 01 ERR
				ret = MapErrorToStatus((*resp).fail[3]);
			else
				ret = OTI_FAILED;
		}
		else
			ret = OTI_UNSUPPORTED;
	}

	if (ret == OTI_COLLISION_ERROR)
	{
		msgLen = PCD_MAX_BUFFER_SIZE;
		ret = RecvMsg(msgBuf, &msgLen, resp, timeout);
		goto afterResp;
	}
	else if (ret != OTI_OK)
	{
		LOG_TRACE("failed with error code %X\r\n", (int)ret);
		return ret;
	}

	/* Try parsing answer nonetheless, even if we get errors */
	memcpy(act_emv_resp->Data, (*resp).succ, (*resp).succLen);

	act_emv_resp->Length = (*resp).succLen;
	act_emv_resp->CurrentLocation = 0;

	return ret;
}


/* Doxygen format:*/
/************************** ActivateTransaction() *******************************************************************//**
* instruct the Saturn to start polling for the card.
*
* If it does not find a card for the time duration specified in the Poll time-out configuration, it will end the transaction. \n
* If it does find a card within the specified time interval, it will attempt to carry out the transaction. \n
* The transaction flow between the Reader and the card depends on the type of transaction and card detected. \n
* If the card is a PayPass M/Chip or qVSDC Card, the reader will use the EMV transaction settings set in the reader. \n
* If the card is a MagStripe Card, the reader will perform an ordinary MSD transaction and return a subset of the results. \n
* If the transaction is completed successfully, and the card was an EMV card,
* then the reader will send back the clearing record information in the response data;
* otherwise, if the card does not support EMV (for example, if it be a PayPass MagStripe Card),
* the reader will send back track information in the response data. \n
* As this function is blocking, CancelTransaction() command must be sent from another task. \n
* The ActivateTransaction() function will handle the cancellation and return
* with the special status OTI_EMV_TRANSACTION_CANCELLED to indicate it was user-interrupted.
*
* @return	 OTI_STATUS				\n
| Value					|	Description																	|
|-----------------------|-------------------------------------------------------------------------------|
| OTI_OK				| Function completed successfully, resp is filled 								|
| OTI_MEMORY_ERROR		| Message buffer couldn't be allocated											|
| OTI_EMV_TRANSACTION_CANCELLED| Transaction was cancelled before completion							|
| OTI_FAILED	    	| Command execution failed on Saturn reader			 							|
| OTI_UNSUPPORTED		| Command not supported by Saturn reader										|

* See MapErrorToStatus() for extra error messages returned by ExchangeCmd()
*
* \ingroup EMV
********************************************************************************************************************/
OTI_STATUS ActivateTransaction(ACT_EMV_STRUCT *act_emv,		//!< [in]  Activate transaction command parameters. See ACT_EMV_STRUCT definition in otilib.h
	TLV_MSG *act_emv_resp,//!< [out] Response of activate transaction. See EMV_PARAMS_STRUCT definition in otilib.h
	PMSG_BUFF msgBuf) //!< [in] Buffer for building message
{
	OTI_STATUS ret;
	RESP_MSG_STRUCT resp;
	;
	WORD respLen = PCD_MAX_BUFFER_SIZE;
	DWORD timeout = 30000;
	static BYTE cmdPollEMV[] = { 0xFD, 0x25, 
		0x9F, 0x02, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x9A, 0x03, 0x00, 0x00, 0x00,
		0x9C, 0x01, 0x00,
		0x5F, 0x2A, 0x02, 0x00, 0x00,
		0x9F, 0x21, 0x03, 0x00, 0x00, 0x00, 
		0xDF, 0x81, 0x69, 0x01, 0x00,
		0x9f, 0x35, 0x01, 0x55 // Transaction categoty code
	};

	/* Make sure the response structure is properly empty */
	//memset(act_emv_resp, 0x00, sizeof(EMV_RESP_STRUCT));

	/* Fill the command fields from the given act_emv */
	memcpy(cmdPollEMV + 5, act_emv->amt_auth, 6);
	memcpy(cmdPollEMV + 13, act_emv->date, 3);
	memcpy(cmdPollEMV + 22, act_emv->currency_code, 2);
	
	if (act_emv->properties & EMV_TRANS_TIME) // Check if "Transaction Time" flag is enabled.
		memcpy(cmdPollEMV + 27, act_emv->trans_time, 3);
	else
		memset(cmdPollEMV + 24, 0x00, 3); // erase transaction type tag and length

	if (act_emv->properties & EMV_TRANS_TYPE) // Check if "Transaction Type" flag is enabled.
		cmdPollEMV[18] = act_emv->trans_type; // set transaction type
	else
		memset(cmdPollEMV + 16, 0x00, 2); // erase transaction type tag and length

	if (act_emv->properties & EMV_TRANS_DUKPT) // Check if "Transaction Type" flag is enabled.
		cmdPollEMV[34] = *(act_emv->dukpt_key); // set transaction type
	else
		memset(cmdPollEMV + 30, 0x00, 5); // erase transaction type tag and length

	LOG_OUTPUT("Please present a card\r\n");
	WORD msgBufLen = msgBuf->Max_Length;
	ret = ExchangeCmd(OP_DO, cmdPollEMV, sizeof(cmdPollEMV), msgBuf->Data, &msgBufLen, &resp, timeout);

	ret = GetResponse(msgBuf->Data, ret, msgBufLen, respLen, &resp, act_emv_resp, timeout);
	return ret;
}// ActivateTransaction ()

OTI_STATUS WaitForFC(TLV_MSG *act_emv_resp,
	PMSG_BUFF msgBuf) //!< [in] Buffer for building message
{
	OTI_STATUS status;
	RESP_MSG_STRUCT resp;
	WORD respLen = PCD_MAX_BUFFER_SIZE;
	DWORD respTimeout = 30000;
	WORD msgBufLen = msgBuf->Max_Length;

	status = RecvMsg(msgBuf->Data, &msgBufLen, &resp, respTimeout);
	if (status != OTI_OK)
		return status;
	status = GetResponse(msgBuf->Data, status, msgBufLen, respLen, &resp, act_emv_resp, respTimeout);
	msgBuf->Actual_Length = msgBufLen;
	return status;
}
// end
