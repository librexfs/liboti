//-------------------------------------------------------------------------------------------------------
// Implements
//-------------------------------------------------------------------------------------------------------
#include "polling_manager.h"

//-------------------------------------------------------------------------------------------------------
// Uses
//-------------------------------------------------------------------------------------------------------
#include "ActivateTransactionParser.h"

static OnOnlineRequest OnlineRequest_Callback;
static OnOfflineRequest OfflineRequest_Callback;

#define TAG_USER_INTERFACE_REQUEST_DATA 0xdf8144
#define TAG_OUTCOME_PARAMETER_SET 0xdf8129
#define TAG_RESULT_TERMINATE_TEMPLATE 0xf2

typedef enum
{
	EMV_MESSAGE_IDENTIFIER_CARD_READ_OK = 0x17,
	EMV_MESSAGE_IDENTIFIER_TRY_AGAIN = 0x21,
	EMV_MESSAGE_IDENTIFIER_APPROVED = 0x3,
	EMV_MESSAGE_IDENTIFIER_APPROVED_SIGNED = 0x1a,
	EMV_MESSAGE_IDENTIFIER_DECLINED = 0x7,
	EMV_MESSAGE_IDENTIFIER_ERROR_OTHER_CARD = 0x1c,
	EMV_MESSAGE_IDENTIFIER_INSERT_CARD = 0x1d,
	EMV_MESSAGE_IDENTIFIER_SEE_PHONE = 0x20,
	EMV_MESSAGE_IDENTIFIER_AUTHORISING_PLEASE_WAIT = 0x1b,
	EMV_MESSAGE_IDENTIFIER_CLEAR_DISPLAY = 0x1e,
	EMV_MESSAGE_IDENTIFIER_N_A = 0xff
} EMV_MESSAGE_IDENTIFIER;

typedef enum {
	EMV_OUTCOME_PARAMETER_STATUS_Approved = 1,
	EMV_OUTCOME_PARAMETER_STATUS_Declined = 2,
	EMV_OUTCOME_PARAMETER_STATUS_Online_Request = 3,
	EMV_OUTCOME_PARAMETER_STATUS_End_Application = 4,
	EMV_OUTCOME_PARAMETER_STATUS_Select_Next = 5,
	EMV_OUTCOME_PARAMETER_STATUS_Try_Another_Intreface = 6,
	EMV_OUTCOME_PARAMETER_STATUS_Try_Again = 7,
} EMV_OUTCOME_PARAMETER_STATUS;

/*
on terminate:
- if UserInterfaceRequestData.MessageIdentifier == TRY_AGAIN || SEE_PHONE
wait for another FC
- if OutcomeParameterSet nibble 1 == 0110: Try Another Intreface || 0111: Try Again
wait for another FC
*/
static XRESULT Is_Reader_Still_Polling(TLV_MSG* ptlv_Termination_Template)
{
	TLV_MSG tlv_selected;

	if (ptlv_Termination_Template == NULL)
		return XERROR;
	if (ptlv_Termination_Template->Data[0] != TAG_RESULT_TERMINATE_TEMPLATE) // Check if F2 response arrived.
	{
		LOG_TRACE("Not starting with F2");
		return XERROR;
	}
	if (!TLV_StepIn(ptlv_Termination_Template, TAG_RESULT_TERMINATE_TEMPLATE)) // Step into the response message.
	{
		LOG_TRACE("Failed to step-in (F2)!\n");
		return XERROR;
	}
	if (TLV_SearchTag(ptlv_Termination_Template, TAG_RESULT_TERMINATE_TEMPLATE, TAG_USER_INTERFACE_REQUEST_DATA, &tlv_selected))
	{
		BYTE y_MessageIdentifier = tlv_selected.Data[0];
		switch (y_MessageIdentifier)
		{
		case EMV_MESSAGE_IDENTIFIER_TRY_AGAIN:
		case EMV_MESSAGE_IDENTIFIER_SEE_PHONE:
			return XTRUE;
		}
	}
	if (TLV_SearchTag(ptlv_Termination_Template, TAG_RESULT_TERMINATE_TEMPLATE, TAG_OUTCOME_PARAMETER_SET, &tlv_selected))
	{
		BYTE status = tlv_selected.Data[0] >> 4;
		switch (status)
		{
		case EMV_OUTCOME_PARAMETER_STATUS_Select_Next:
		case EMV_OUTCOME_PARAMETER_STATUS_Try_Another_Intreface:
		case EMV_OUTCOME_PARAMETER_STATUS_Try_Again:
			return XTRUE;
		}
	}
	if (TLV_SearchTag(ptlv_Termination_Template, TAG_RESULT_TERMINATE_TEMPLATE, 0xdf68, &tlv_selected))
	{
		switch (tlv_selected.Data[0])
		{
		case 6: // Collision
			return XTRUE;
		}
	}
	return XFALSE; // PCD is not polling anymore
}

static OTI_STATUS Poll(ACT_EMV_STRUCT *act_trans, TLV_MSG *resp)
{
	BYTE msg[900];
	MSG_BUFF msgBuf;

	msgBuf.Data = msg;
	msgBuf.Max_Length = 900;
	
	TLV_MSG dataHolder;
	XRESULT xresult;
	static BYTE dataBuffer[MAX_BUFFER_SIZE];
	dataHolder.Data = dataBuffer;
	dataHolder.CurrentLocation = 0;
	dataHolder.Length = MAX_BUFFER_SIZE;

	OTI_STATUS waitForFcStatus;

	if ((waitForFcStatus = ActivateTransaction(act_trans, resp, &msgBuf)) == OTI_OK)
		return OTI_OK;

	// determine if Reader is still polling

	while (true)
	{
		xresult = Is_Reader_Still_Polling(resp);
		switch (xresult)
		{
		case XFALSE:
			return OTI_FAILED;	//not polling anymore		
		case XTRUE:
			break;
		case XERROR:	// no F2
		default:
			return OTI_ERROR;
		}

		LOG_OUTPUT("Try agian.\r\n");
		LOG_OUTPUT("Please present a card\r\n");
		waitForFcStatus = WaitForFC(resp, &msgBuf);
		switch (waitForFcStatus)
		{
		case OTI_OK:
			return OTI_OK;
		case OTI_FAILED:
			continue;
		default:
			LOG_OUTPUT("Failed to get FC tag from reader.\n");
			return waitForFcStatus;
		}
	}
}

static void PrintTags(TLV_MSG *resp)
{
	int tag;
	TLV_MSG dataHolder;
	static BYTE dataBuffer[MAX_BUFFER_SIZE];
	dataHolder.Data = dataBuffer;
	dataHolder.CurrentLocation = 0;
	dataHolder.Length = MAX_BUFFER_SIZE;

	LOG_OUTPUT("Transaction Result Tags:\n");

	while (TLV_ReadTag(resp, &tag, &dataHolder))
	{
		LOG_OUTPUT("Tag: 0x%.6X\tData:", tag);
		LOG_OUTPUT_BUF(dataHolder.Data, dataHolder.Length);
	}
}

BOOLEAN HandleOnline(TLV_MSG *resp)
{
	BYTE msg[900];
	MSG_BUFF msgBuf;

	msgBuf.Data = msg;
	msgBuf.Max_Length = 900;

	TLV_MSG incomingE8;
	BYTE incomingBuffer[MAX_BUFFER_SIZE];
	incomingE8.Data = incomingBuffer;
	incomingE8.Length = MAX_BUFFER_SIZE;
	incomingE8.CurrentLocation = 0;

	LOG_OUTPUT("Online: Connecting with PSP...\r\n");

	if (OnlineRequest_Callback(resp, &incomingE8))
	{
		LOG_OUTPUT("Sending approval to PCD...\r\n");
		SAT_RETRY(OnlineStatus(OTI_STAT_SUCCEEDED, &msgBuf));
		return TRUE;
	}
	else
	{
		LOG_OUTPUT("Sending failure to PCD...\r\n");
		SAT_RETRY(OnlineStatus(OTI_STAT_FAILED, &msgBuf));
		return FALSE;
	}
}

void HandleOffline(TLV_MSG *resp)
{
	LOG_OUTPUT("Offline: Connecting with PSP...\r\n");

	OfflineRequest_Callback(resp);

	LOG_OUTPUT("Sending approval to PCD...\r\n");
}

XRESULT DoTransaction(ACT_EMV_STRUCT *act_trans, TLV_MSG *resp)
{
	TLV_MSG dataHolder;
	static BYTE dataBuffer[MAX_BUFFER_SIZE];
	dataHolder.Data = dataBuffer;
	dataHolder.CurrentLocation = 0;
	dataHolder.Length = MAX_BUFFER_SIZE;

	OTI_STATUS status = Poll(act_trans, resp);
	if (status == OTI_FAILED)
		return XFALSE; // not polling equals Decline?
	if (status == OTI_OK)
	{
		if (!TLV_StepIn(resp, 0xfc))
		{
			LOG_TRACE("Failed to step-in (fc)!\n");
			return XERROR;
		}

		LOG_OUTPUT("PollEMV Success\n");
		PrintTags(resp);

		if (TLV_SearchTag(resp, 0xfc, 0xdf69, &dataHolder)) // PCD Transaction Result
		{
			if (dataHolder.Data[0] == 0x80) // Online
			{
				return HandleOnline(resp);
			}
			else if (dataHolder.Data[0] == 0x40) // Offline
			{
				HandleOffline(resp);
				return TRUE;
			}

			return XFALSE; //DECLINE
		}
		else
		{
			LOG_OUTPUT("FAILED TO FIND 'PCD Transaction Result' (DF 69) TAG\n");
			return XERROR;
		}
	}
	else
	{
		LOG_OUTPUT("PollEMV failed\r\n");

		if (!TLV_StepIn(resp, TAG_RESULT_TERMINATE_TEMPLATE))
		{
			LOG_TRACE("Failed to step-in (fc)!\n");
			return XERROR;
		}
		PrintTags(resp);
		return XFALSE;
	}
}

void InitTransaction(OnOnlineRequest callbackOnline, OnOfflineRequest callbackOffline)
{
	OnlineRequest_Callback = callbackOnline;
	OfflineRequest_Callback = callbackOffline;
}
