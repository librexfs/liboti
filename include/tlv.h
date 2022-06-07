/**
                         OTI HostPCD Protocol C Library

          **********************************************************************
          **        Copyright (c) 2008-2015 On Track Innovations Ltd.         **
          ** This computer program includes confidential, proprietary         **
          ** information and is a trade secret of On Track Innovations Ltd.   **
          ** All use, disclosure, and/or reproduction is prohibited           **
          ** unless expressly authorized in writing.                          **
          **                    All rights reserved.                          **
          **********************************************************************

    Description:    Definitions of a TLV data structure and operations on it.
    Author:         On Track Innovations Ltd.
	Version:        4.0.00

*/

#ifndef __OTITLV_H__
#define __OTITLV_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "otilib.h"
#include "protocol.h"
#include "config.h"

#define TLV_LEN_ERROR   ((WORD)0xFFFF)
#define TLV_TAG_ERROR   ((WORD)0xFFFF)

/* OTI Implementation of TLV objects */
typedef struct {
	BYTE   *rawTlv;		/**< Raw bytes of TLV data. */
	WORD    size;		/**< Size of rawTlv data. */
	DWORD   tag;		/**< Integer representation of TAG. */
	WORD    len;		/**< Integer representation of LENGTH and actual size of value. */
	BYTE   *value;	    /**< Pointer to first byte of VALUE in raw Tlv array. */
	BOOLEAN light;	    /**< Indicates whether rawTlv points to locally allocated or external array. */
#ifdef __GNUC__
} __attribute__((packed)) OTI_TLV;
#else
} OTI_TLV;
#endif	// __GNUC__

/**
 * @brief	Initializes a newly created Tlv object with safe empty fields.
 *
 * @param	tlv			[IN] Pointer to the Tlv to initialize.
 *
 * @return	The function returns TRUE if everything went well, FALSE otherwise.
 */
BOOLEAN InitTlv(OTI_TLV *tlv);

/**
 * @brief	Creates a full TLV object from Tag, Len and Value data. This
 *          function takes TLV data and packs it into an encoded BYTE array.
 *
 * @param	tag			[IN] Integer value of the TAG field of the TLV to create.
 * @param   len         [IN] Integer value for the LEN field of the TLV to create
 *                      (and actual length of the VALUE field).
 * @param   value       [IN] Pointer to a BYTE array containing the VALUE field
 *                      of the TLV to create. This array must be of length _len_
 *                      (see parameter above).
 * @param   buf         [IN] this parameter is a pointer to the
 *                      buffer where the raw TLV bytes will be stored. 
 * @param   bufLen      [IN] If _buf_ is not NULL, this parameter specifies its
 *                      size.
 *
 * @return	The function returns a pointer to a newly created and initialized
 *          TLV object. This object's fields are set to _tag_, _len_ and _value_
 *          respectively. The TLV's _rawTlv_ member points to a BYTE array
 *          representing the full encoded TLV, the full size of which is stored
 *          in the _size_ member.
 *
 *          In case of error, the function returns FALSE
 *
 */
BOOLEAN BuildTlv(DWORD tag, WORD len, BYTE *value, BYTE *buf, WORD bufLen, OTI_TLV *tlv);

/**
 * @brief	Creates a deep or shallow copy of an existing TLV object.
 *
 * @param	fromTlv		[IN] Pointer to the Tlv to copy.
 * @param	shallow		[IN] Boolean value indicating whether to force shallow
 *                      copy or not. If FALSE, a deep copy of the original Tlv
 *                      will be created. A deep copy means that the original TLV's
 *                      raw BYTE array is entirely copied into the newly created
 *                      Tlv. Otherwise, the new Tlv only keeps a pointer to the
 *						original one's BYTE array.
 *
 * @return	The function returns a pointer to a newly created TLV object
 *          initialized from the original one.
 *
 *          The calling function MUST FREE that TLV object after use.
 */
OTI_TLV *BuildTlvFromTlv(OTI_TLV *fromTlv, BOOLEAN shallow);

/**
 * @brief   Creates a full TLV object from a string of raw bytes. This
 *          function takes a raw existing TLV as a byte-array and parses its fields.
 *
 * @param   data        [IN] Pointer to the BYTE array containing the raw TLV bytes.
 * @param   dataLen     [IN] Size of the _data_ array.
 * @param   buf         [IN] this parameter is a pointer to the
 *                      buffer where the raw TLV bytes will be stored. If NULL,
 *                      the TLV bytes will be stored in a dynamically allocated
 *                      byte array. If _buf_ points to the same address as _data_,
 *                      a lightweight TLV will be created, and the existing _data_
 *                      buffer will be internally used by the TLV object. Thus it's
 *                      legal to pass the same pointer for _data_ and _buf_ if you
 *                      know your _data_ buffer will remain constant and know
 *                      what you're doing.
 * @param   bufLen      [IN] If _buf_ is not NULL, this parameter specifies its
 *                      size.
 *
 * @return  The function returns a pointer to a newly created and initialized
 *          TLV object. This object's fields are set to _tag_, _len_ and _value_
 *          respectively. The TLV's _rawTlv_ member points to a BYTE array
 *          representing the full encoded TLV, the full size of which is stored
 *          in the _size_ member.
 *
 *          In case of error, the function returns FALSE.
 *
 */
BOOLEAN BuildTlvFromRaw(BYTE *data, WORD dataLen, BYTE *buf, WORD bufLen, OTI_TLV *tlv);

/**
 * @brief   Destroys a TLV object and releases allocated memory it was using.
 *
 * @param   tlv     [IN] Pointer to the Tlv to destroy.
 *
 * @return  The function returns TRUE if everything went well, FALSE otherwise.
 *
 *          This function MUST be called at some point for every created TLV.
 */
//BOOLEAN //DeleteTlv(OTI_TLV *tlv);

/**
 * @brief   Retrieves TLV data fields (tag, length and value) from a raw BYTE array.
 *          This function is provided for convenience and is used internally by
 *          BuildTlvFromRaw().
 *
 * @param   tag         [OUT] WORD containing the integer value of the TLV's tag.
 * @param   len         [OUT] WORD containing the TLV value's length.
 * @param   rawBuf      [IN/OUT] If not NULL, this parameter is a pointer to the
 *                      buffer where the raw TLV bytes will be stored (in which
 *                      case _rawBufLen_ must be set and indicate this buffer's
 *                      size). If NULL, the TLV bytes will be stored in a
 *                      dynamically allocated byte array whose address will be
 *                      returned in that parameter.
 * @param   rawBufLen   [IN/OUT] If _rawBufLen_ is not NULL, this parameter
 *                      specifies the buffer's size.
 * @param   dataPtr     [IN/OUT] Address of a pointer to the buffer where the
 *                      raw TLV data is located. After the function returns,
 *                      that pointer will be positioned right after the last
 *                      byte of TLV data in that buffer.
 * @param   dataLen     [IN] Size of the _dataLen_ buffer.
 * @param   readBytes   [OUT] If not NULL, this parameter will contain the actual
 *                      number of processed bytes.
 *
 * @return  The function returns TRUE if everything went well, FALSE otherwise.
 */
BOOLEAN ParseRawTlvDataToBuffer(DWORD *tag, WORD *len, BYTE **rawBuf, WORD *rawBufLen, BYTE **dataPtr, WORD dataLen, WORD *readBytes);

/**
 * @brief	Encodes TLV data and stores it into a BYTE array. This function is
 *          provided for convenience and is used internally by BuildTlv(). It's
 *          also useful to create TLVs and store them in a static buffer on the
 *          fly.
 *
 * @param   dstBuf      [IN/OUT] Address of a pointer to the buffer where the
 *                      encoded TLV data will be stored. After the function
 *                      returns, that pointer will be positioned right after the
 *                      last byte of TLV data in the destination buffer.
 * @param   dstBufLen   [IN] Size of the _dstBuf_ BYTE array.
 * @param	tag			[IN] Integer value of the TAG field of the TLV to create.
 * @param   len         [IN] Integer value for the LEN field of the TLV to create
 *                      (and actual length of the VALUE field).
 * @param   value       [IN] Pointer to a BYTE array containing the VALUE field
 *                      of the TLV to create. This array must be of length _len_
 *                      (see parameter above).
 * @param   writtenBytes [OUT] If not NULL, this parameter will contain the
 *                      number of bytes written after the function returns.
 *
 * @return	The function returns TRUE if everything went well, FALSE otherwise.
 */
BOOLEAN PackTlvDataToBuffer(BYTE **dstBuf, WORD dstBufLen, DWORD tag, WORD len, BYTE *value, WORD *writtenBytes);

/**
 * @brief Given the integer value of a Tlv tag, indicate how many bytes are
 *        needed to encode it (usually only 1 or 2)
 *
 * @param	tag		[IN] Integer value of the tag to encode (i.e. 0x9F25)
 *
 * @return	The function returns the number of bytes on which the Tag would be
 *          encoded.
 */
BYTE TlvTagSize(DWORD tag);

/**
 * @brief Given the integer value of a Tlv length, indicate how many bytes are
 *        needed to encode it (from 1 to 3)
 *
 * @param	len		[IN] Integer value of the length to encode (i.e. 0x0123)
 *
 * @return	The function returns the number of bytes on which the Length would be
 *          encoded.
 */
BYTE TlvLengthSize(WORD len);

/**
 * @brief Parses part of a BYTE array to convert TAG bytes into an integer value.
 *
 * @param	dataPtr		[IN/OUT] Address of the pointer on the beginning of the
 *                      BYTE array containing the tag to parse. When the function
 *                      returns, *dataPtr points on the first byte following the
 *                      parsed tag in the array being read.
 * @param	endPtr		[IN] Pointer on the last element in the char array being
 *                      parsed,	used for bounds checking.
 * @param	readBytes	[OUT] If not NULL, this variable will contain the number
 *                      of bytes processed during parsing -- ie. by how much
 *                      *dataPtr was increased.
 *
 * @return	The function returns a WORD containing the integer value of the
 *          parsed tag or TLV_TAG_ERROR if an error occurred during parsing.
 */
DWORD ParseTag(BYTE **dataPtr, BYTE *endPtr, BYTE *readBytes);

/**
 * @brief Parses part of a BYTE array to convert LENGTH bytes into an integer
 *        value.
 *
 * @param	dataPtr		[IN/OUT] Address of the pointer on the beginning of the
 *                      BYTE array containing the TLV length to parse. When the
 *                      function returns, *dataPtr points on the first byte
 *                      following the parsed length in the array being read.
 * @param	endPtr		[IN] Pointer on the last element in the char array being
 *                      parsed, used for bounds checking.
 * @param	readBytes	[OUT] If not NULL, this variable will contain the number of bytes
 *						processed during parsing -- ie. by how much *dataPtr was increased.
 *
 * @return	The function returns a WORD containing the integer value of the
 *          parsed length or TLV_LEN_ERROR if an error occurred during parsing.
 */
WORD ParseLen(BYTE **dataPtr, BYTE *endPtr, BYTE *readBytes);

/**
 * @brief	Encode given TAG according to Tlv rules and store the resulting bytes into a previously
 *			allocated array, with bounds checking.
 *
 * @param	tag			[IN] The integer value of the TAG to encode. It should comply with Tlv rules
 *						regarding tags as no checking is done.
 * @param	bufPtr		[IN/OUT] Address of the pointer on the beginning of the BYTE array where
 *						the resulting encoded TAG will be stored. When the function returns, *bufPtr
 *						points on the first byte following the encoded TAG in the destination array.
 * @param	endPtr		[IN] Pointer on the last element in the destination array, used for bounds
 *						checking.
 * @param	writtenBytes	[OUT] If not NULL, this variable will contain the number of bytes written
 *							during the packing process -- ie. by how much *bufPtr was increased.
 *
 * @return	The function returns TRUE if everything went well, FALSE otherwise.
 */
BOOLEAN PackTag(DWORD tag, BYTE **bufPtr, BYTE *endPtr, BYTE *writtenBytes);

/**
 * @brief	Encode given LENGTH according to Tlv rules and store the resulting chars into a previously
 *			allocated array, with bounds checking.
 *
 * @param	len			[IN] The integer value of the LENGTH to encode.
 * @param	bufPtr		[IN/OUT] Address of the pointer on the beginning of the char array where
 *						the resulting encoded LENGTH will be stored. When the function returns, *bufPtr
 *						points on the first byte following the encoded LENGTH in the destination array.
 * @param	endPtr		[IN] Pointer on the last element in the destination array, used for bounds
 *						checking.
 * @param	writtenBytes	[OUT] If not NULL, this variable will contain the number of bytes written
 *							during the packing process -- ie. by how much *bufPtr was increased.
 *
 * @return	The function returns TRUE if everything went well, FALSE otherwise.
 */
BOOLEAN PackLen(WORD len, BYTE **bufPtr, BYTE *endPtr, BYTE *writtenBytes);

/**
 */
BOOLEAN ExtractTLV(BYTE *data, WORD dataLen, BOOLEAN shallow, WORD *path, BYTE pathLen, OTI_TLV *tlv);

/**
 * @brief	Tests the TLVLIB module by creating/parsing TLVs and checking that
 *          the resulting objects are as expected.
 *
 *          There are 16 tests, the results of which will be printed out with specific
 *          description codes explained below:
 *
 *              T<n> -> TAG<size>
 *              L<n> -> LEN<size>
 *              V<n> -> VALUE<size>
 *              D    -> DYNAMICALLY ALLOCATED
 *              S    -> USING STATIC BUFFER
 *			    R	 -> FROM RAW BYTES
 *
 *          NOTE: These tests will only be run if the OTI_DEBUG_TLV macro is defined.
 *                The function's test code will not be compiled otherwise and will simply
 *                display "TLV TESTING DISABLED" when called.
 *
 * @return	The function returns OTI_OK if everything went well, an error code
 *          otherwise.
 */
OTI_STATUS TestTLV(void);

#endif // __OTITLV_H__
