#include "ActivateTransactionParser.h"

int Parse_Len(TLV_MSG *buffer, int *data)
{
	BYTE firstByte = *(buffer->Data + buffer->CurrentLocation);
	if (firstByte <= 0x7F)
	{
		*data = *(buffer->Data + buffer->CurrentLocation);
		buffer->CurrentLocation += 1;
	}
	else if (firstByte == 0x81)
	{
		buffer->CurrentLocation += 1;
		*data = *(buffer->Data + buffer->CurrentLocation);
		buffer->CurrentLocation += 1;
	}
	else if (firstByte == 0x82)
	{
		buffer->CurrentLocation += 1;
		*data = *(buffer->Data + buffer->CurrentLocation) << 8;
		buffer->CurrentLocation += 1;
		*data = *data | *(buffer->Data + buffer->CurrentLocation);
		buffer->CurrentLocation += 1;
	}
	else
	{
		return 1;
	}
	return 0;
}

BOOLEAN Parse_Tag(TLV_MSG *buffer, int *tag)
{
	int numOfBytes = 1;
	if (buffer->CurrentLocation >= buffer->Length)
		return FALSE; // Nothing to read.

	if ((buffer->Data[buffer->CurrentLocation] & 0x1F) == 0x1F)
	{
		++numOfBytes;
		if ((buffer->Data[buffer->CurrentLocation + 1] & 0x80) == 0x80)
			++numOfBytes;
	}

	*tag = 0;
	switch (numOfBytes)
	{
	case 3:
		*tag = buffer->Data[(buffer->CurrentLocation)++] << 16;
		*tag |= buffer->Data[(buffer->CurrentLocation)++] << 8;
		*tag |= buffer->Data[(buffer->CurrentLocation)++];
		break;
	case 2:
		*tag = buffer->Data[(buffer->CurrentLocation)++] << 8;
		*tag |= buffer->Data[(buffer->CurrentLocation)++];
		break;
	case 1:
		*tag = buffer->Data[(buffer->CurrentLocation)++];
		break;
	default:
		return FALSE;
	}

	return TRUE;
}

BOOLEAN Parse_Data(TLV_MSG *buffer, TLV_MSG *outData)
{
	int len;
	if (Parse_Len(buffer, &len))
	{
		LOG_DEBUG("UNABLE TO PARSE LEN OF TAG\n");
		return FALSE;
	}

	if (len == 0)
	{
		outData->Length = outData->CurrentLocation = 0;
		outData->Data = NULL;
	}
	else
	{
		outData->Data = buffer->Data + buffer->CurrentLocation;
		outData->CurrentLocation = 0;
		outData->Length = len;

		buffer->CurrentLocation += len;
	}

	return TRUE; // Success.
}

BOOLEAN TLV_StepIn(TLV_MSG *buffer, int tag)
{
	int msg_len;

	ResetPosition(buffer); // Reset the data pointer location (CurrentLocation) to the first byte.

	/* Parse every TLV in the response tag FC. */
	if (*(buffer->Data + (buffer->CurrentLocation++)) != tag)
	{
		LOG_DEBUG("WRONG TEMPLATE IN RESP\n");
		return FALSE;
	}

	if (Parse_Len(buffer, &msg_len))
	{
		LOG_DEBUG("UNABLE TO PARSE LENGHT\n");
		return FALSE;
	}

	if (buffer->CurrentLocation + msg_len != buffer->Length)
	{
		LOG_DEBUG("TLV LEN IS NOT MATCH ACTUAL DATA\n");
		return FALSE;
	}

	return TRUE;
}

BOOLEAN TLV_ReadTag(TLV_MSG *buffer, int *tag, TLV_MSG *data)
{
	if (!Parse_Tag(buffer, tag))
	{
		/*LOG_DEBUG("UNABLE TO PARSE TAG");*/
		return FALSE; // Error while reading the next tag.
	}

	if (!Parse_Data(buffer, data))
	{
		LOG_DEBUG("UNABLE TO PARSE DATA\n");
		return FALSE;
	}

	return TRUE; // success
}

void ResetPosition(TLV_MSG *buffer)
{
	buffer->CurrentLocation = 0;
}

BOOLEAN TLV_SearchTag(TLV_MSG *buffer, int tlv_tag, int tagToFind, TLV_MSG *data)
{
	int curTag;
	ResetPosition(buffer);
	if (!TLV_StepIn(buffer, tlv_tag))
	{
		LOG_DEBUG("FAILED TO STEP IN\n");
		return FALSE;
	}

	while (TLV_ReadTag(buffer, &curTag, data))
	{
		if (curTag == tagToFind)
			return TRUE;
	}

	return FALSE;
}