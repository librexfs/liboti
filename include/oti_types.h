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
#ifndef OTI_TYPES_H_
#define OTI_TYPES_H_

typedef unsigned char UCHAR;
typedef unsigned char BYTE;      // 1 byte
typedef BYTE BOOLEAN;
typedef unsigned int UINT;
typedef unsigned short WORD;      // 2 bytes
typedef unsigned long ULONG;
typedef unsigned long DWORD;     // 4 bytes

#ifndef TRUE
#define TRUE    (BOOLEAN)1
#endif
#ifndef FALSE
#define FALSE   (BOOLEAN)0
#endif
#define NONE    2  // Special case for 'unitialized' boolean

typedef enum {
	XFALSE,
	XTRUE,
	XERROR
} XRESULT;

typedef struct
{
	BYTE *Data;
	int Length;
	int CurrentLocation;
} TLV_MSG;


typedef struct
{
	BYTE *Data;
	int Actual_Length;
	int Max_Length;
} MSG_BUFF;

typedef MSG_BUFF* PMSG_BUFF;

typedef enum
{
	TRACE,
	DEBUG,
	OUTPUT
} log_level;

/* Saturn Configuration structure saved in OTI file */
typedef enum {
	BPS_50 = 1,
	BPS_75,
	BPS_110,
	BPS_134,
	BPS_150,
	BPS_200,
	BPS_300,
	BPS_600,
	BPS_1200,
	BPS_1800,
	BPS_2400,
	BPS_4800,
	BPS_9600,	//!< Default baud rate
	BPS_19200,
	BPS_38400,
	BPS_57600,
	BPS_115200,
	BPS_230400,
	BPS_LAST
} BAUD_TYPE;

typedef enum {
	ONE_STOPBIT, TWO_STOPBITS
} STOP_TYPE;

typedef struct {
	BYTE port[1024];		//!< Path to "/dev/tty*" file in Linux or "\\.\COM*" port in Windows
	BAUD_TYPE baud_rate;
	STOP_TYPE stop_bits;
	DWORD poll_timeout;		//!< Specific timeout for PollEMV in miliseconds, used in the ActivateTransaction() function.
	BOOLEAN debug;			//!< Debug on/off (default is off)
} OTI_CONFIG_STRUCT;

typedef enum {
	OTI_ERROR = 0,
	OTI_OK,
	OTI_UNSUPPORTED,
	OTI_FAILED,
	OTI_NAK,
	OTI_DATA_MISSING,
	OTI_LRC_ERROR,
	OTI_TLV_ERROR,
	OTI_INVALID_PARAM,
	OTI_STX_MISSING,
	OTI_ETX_MISSING,
	OTI_UNKNOWN_OPCODE,
	OTI_TOO_MANY_INSTRUCTIONS,
	OTI_PARAM_NOT_SUPPORTED,
	OTI_MEMORY_ERROR,
	OTI_BUFFER_OVERFLOW,
	OTI_COM_ERROR,
	OTI_COM_TIMEOUT,
	OTI_CONFIG_ERROR,
	OTI_INVALID_DATA,
	OTI_INCOMPLETE_DATA,
	OTI_UNKNOWN_DATA,
	OTI_COLLISION_ERROR,
	OTI_PCD_MEM_WRITE_ERROR,
	OTI_PCD_MEM_READ_ERROR,
	OTI_RF_FAILURE,
	OTI_RF_FAILURE_AFTER_RECOVERY,
	OTI_DATA_LENGTH_OVERFLOW,
	OTI_HERCULES_ERROR_CODE,
	OTI_SAM_CONTACT_FAILURE,
	OTI_SAM_MISSING,
	OTI_SAM_NOT_ACTIVE,
	OTI_SAM_PROTOCOL_ERROR,
	OTI_MISSING_PARAM,
	OTI_APPLI_DISABLED,
	OTI_SUB_SCHEME_DISABLED,
	OTI_INDEX_NOT_FOUND,
	OTI_KEY_HASH_MISMATCH,
	OTI_AID_LENGTH_ERROR,
	OTI_DATA_LENGTH_ERROR,
	OTI_PCD_SETUP_ERROR,
	OTI_SUBSCHEME_NOT_APPLICABLE,
	OTI_PKI_STORAGE_ERROR,
	OTI_EMV_TRANSACTION_TERMINATED,
	OTI_POLL_TIMEOUT,
	OTI_EMV_TRANSACTION_CANCELLED,
	OTI_READER_BUSY,
	OTI_BOOT_CRC_ERROR,
	OTI_BOOT_PACKET_SEQ_ERROR,
	OTI_BOOT_TARGET_ID_ERROR,
	OTI_BOOT_PACKET_LEN_ERROR,
	OTI_BOOT_FILE_TOO_BIG,
	OTI_BOOT_STAMP_ERROR,
	OTI_BOOL_FILE_TARGET_ERROR,
	OTI_BOOT_KEY_VERSION_ERROR,
	OTI_BOOT_HASH_ALGO_ERROR,
	OTI_BOOT_META_NOT_ORIGINAL,
	OTI_BOOT_META_MISS_MANDATORY,
	OTI_BOOT_META_TLV_ERROR,
	OTI_BOOT_META_LEN_ERROR,
	OTI_BOOT_META_VERSION_ERROR,
	OTI_PARAM_MISSING,
	OTI_INSUFFICIENT_CREDIT,
	OTI_BUFFER_LENGTH
} OTI_STATUS;

typedef enum {
	POLL_TIMEOUT = 1,
	ONLINE_AUTH_TIMEOUT = 3,
	POST_TRANS_MSG_TIMEOUT,
	BETWEEN_POLL_TIMEOUT,
} TIMEOUT;

#ifndef min
#define min(a, b) ((a) < (b) ? (a) : (b))
#endif

#define BUFTOINT(s)    (((BYTE)(*(s))<<8)|((BYTE)(*(s+1))))
#define BUFTOLONG(s)   (((DWORD)(*(s))<<24)|((DWORD)(*(s+1))<<16) \
                          |((DWORD)(*(s+2))<<8)|((DWORD)(*(s+3))))
#define LONGTOBUF(i,o) (*((o)+3)=(BYTE)(i),*((o)+2)=(BYTE)((i)>>8),  \
                        *((o)+1)=(BYTE)((i)>>16),*(o)=(BYTE)((i)>>24) )
#define INTTOBUF(i,o)  (*((o)+1)=(BYTE)(i),*(o)=(BYTE)((i)>>8))
#define DWORDTOWORD(d) ((WORD)min(d, 0xFFFF))

/* Global constants */
#define AID_MAXLGTH                 16
#define DOL_MAXLGTH                 50
#define LCD_MAXLGTH                 20
#define KEY_MAXLGTH                 300
#define AID_LIST_LENGTH             10
#define TIMEOUT_LIST_LENGTH         4
#define PCD_MAX_RESPONSE_BUFFER_SIZE         50
#define PCD_MAX_BUFFER_SIZE         900
#define PCD_BOOT_MAX_BUFFER_SIZE    1070
#define PCD_BOOT_MAX_PACKET_SIZE    1024
#define PCD_BOOT_MAX_RETRIES        3
#define POLL_TIMEOUT_DEFAULT        16000
#define MAX_COMM_TIMEOUT            0x124F80    //!<  20 minutes
#define EMV_RES_TAGS_LIST_MAXLGTH   120         //!<  without encapsulating tag
#define MS_RES_TAGS_LIST_MAXLGTH    70          //!<  without encapsulating tag
/* YMODEM protocol defines for Firmware Upload */
#define YM_LONG_FRAME_SIZE  1024
#define YM_SHORT_FRAME_SIZE 128
#define YM_SOH              0x01
#define YM_STX              0x02
#define YM_ACK              0x06
#define YM_NAK              0x15
#define YM_EOT              0x04
#define YM_REQC             0x43

/* LCD Message formats */
/**  \defgroup LCD_MSG_FORMAT LCD Message formats 
*  @ingroup DEFINES
*  @{*/
#define CENTER_X        0x40
#define CENTER_Y        0x80    //!< Used internally by GetLCDMessage
#define FONT_EN_SMALL   0x00    //!< Arial, Latin 1, size 8
#define FONT_EN_MEDIUM  0x01    //!< Arial, Latin 1, size 10
#define FONT_EN_LARGE   0x02    //!< Arial, Latin 1, size 12
#define FONT_CNTRL_EUR  0x03    //!< Tahoma, Latin 2, size 10
#define FONT_CYRILLIC   0x04    //!< Tahoma, cyrillic, size 10
/**@}*/

/* Modify lines bitmaps */
/**  \defgroup LINES_BMP Modify lines bitmaps
*  @ingroup DEFINES
*  @{*/
#define LCD_LINE1       0x01
#define LCD_LINE2       0x02
#define LCD_LINE3       0x04
#define LCD_LINE_ALL    (LCD_LINE1 | LCD_LINE2 | LCD_LINE3)
/**@}*/

/* SelectLCD actions */
typedef enum {
	LCD_DISPLAY = 0, LCD_CHANGE_DEFAULT, LCD_GET_DEFAULT
} LCDMSG_SELECT_ACT;

/* SelectLCD Messages */
typedef enum {
	IDLE_MSG = 0,
	PRESENT_CARD_MSG,
	TRANS_SUCCESS_MSG,
	POLL_AFTER_COLLISION_MSG,
	REMOVAL_MSG,
	COLLISION_ERROR_MSG,
	APPROVED_ONLINE_MSG,
	APPROVED_OFFLINE_MSG,
	DECLINED_OFFLINE_MSG,
	DECLINED_ONLINE_MSG,
	TRANS_TERMINATED_MSG,
	PIN_ENTRY_MSG,
	RFU_0C_MSG,
	RFU_0D_MSG,
	RFU_0E_MSG,
	RFU_0F_MSG,
	RFU_10_MSG,
	WELCOME_MSG,
	THANK_YOU_MSG,
	THANK_YOU_NAME_MSG,
	RFU_14_MSG,
	USE_OTHER_CARD_MSG,
	RFU_16_MSG,
	RFU_17_MSG,
	RFU_18_MSG,
	TRY_AGAIN_MSG,
	INTERNATIONAL_CARD_SWIPE_MSG,
	RFU_1B_MSG,
	RFU_1C_MSG,
	RFU_1D_MSG,
	TIMEOUT_INSERT_SWIPE_MSG,
	RFU_1F_MSG,
	RFU_20_MSG,
	RFU_21_MSG,
	RFU_22_MSG,
	OK_REMOVE_CARD_MSG,
	PROCESSING_MSG,
	COMM_FAILED_MSG,
	INSERT_OR_SWIPE_MSG,
	READ_ERROR_MSG,
	CURRENT_MSG = 0xFF
} LCD_MESSAGE;

typedef struct {
	LCD_MESSAGE index;			//!< index to LCD_MESSAGE. 
	BOOLEAN yCenter;			//!< TRUE if the message is supposed be vertically centered on the display
	BYTE setLines;				/**< A bit field describing which lines of the message should be affected by the change. 
									It can be an OR combination of the LINES_BMP .
									For instance, if set to LCD_LINE1 | LCD_LINE3, the modified message middle line will remain unchanged. \n
									To set a message on only one or two lines, the remaining empty lines must explicitly be set to an empty string. */
	BYTE format[3];				/**< An array of OR formatting values (centering | font), each cell of the array defines the LCD_MSG_FORMAT 
								    for a line of text (cell 0, 1 or 2 respectively for lines 1, 2 and 3). */
	BYTE L1[LCD_MAXLGTH + 1];
	BYTE L2[LCD_MAXLGTH + 1];
	BYTE L3[LCD_MAXLGTH + 1];
} LCDMSG_SETTINGS_STRUCT;

/* Outport parameters */
typedef enum {
	OUT_PCD = 0,    //!< Leds/Beeps/LCD handled by the reader automatically
	OUT_HOST,       //!< Leds/Beeps/LCD handled by the host terminal
	OUT_HOST_LCD    //!< Leds/Beeps handled by reader, LCD handled by the host terminal
} OTI_OUTPORT;

#define OUT_ALL_OFF     0x00
#define OUT_BUZZER_ON   0x01
#define OUT_LED1_ON     0x02
#define OUT_LED2_ON     0x04
#define OUT_LED3_ON     0x08
#define OUT_LED4_ON     0x10
#define OUT_BKLGHT_ON   0x20
#define OUT_LED_ALL_ON  OUT_LED1_ON | OUT_LED2_ON | OUT_LED3_ON | OUT_LED4_ON

/* Graphics Display Commands */
#define GFX_SET_PEN     0x06
#define GFX_SET_POS     0x07
#define GFX_CLEAR       0x08
#define GFX_CRLF        0x0A
#define GFX_SET_FONT    0x0C
#define GFX_LINE        0x0E
#define GFX_BOX         0x0F
#define GFX_FILLBOX     0x10

/* Graphics Display types */
typedef enum {
	GFX_BLACK = 0, GFX_WHITE, GFX_INVERSE
} GFX_PEN;

typedef enum {
	GFX_SMALL = 0, GFX_MEDIUM, GFX_BIG
} GFX_FONT;

/* Graphics Display limits */
#define GFX_MAX_X       0x9F
#define GFX_MAX_Y       0x3F

/* AID representation */
typedef struct {
	BYTE aid_len;
	BYTE aid[AID_MAXLGTH];
} AID;

/* Internal reader timeouts */
typedef struct {
	BYTE index;
	BYTE timeout[2];
} TIMEOUTS;

/* Bitfield values for properties set in AID_PARAM structure */
#define AID_PARAMS_CURRENCY             0x00000001
#define AID_PARAMS_TERM_TYPE            0x00000002
#define AID_PARAMS_APP_VERSION          0x00000004
#define AID_PARAMS_FLOOR_LIMIT          0x00000008
#define AID_PARAMS_TERM_CAP             0x00000010
#define AID_PARAMS_TARGET_PERC          0x00000020
#define AID_PARAMS_MAX_TARGET_PERC      0x00000040
#define AID_PARAMS_THRESHOLD            0x00000080
#define AID_PARAMS_TAC                  0x00000100
#define AID_PARAMS_DDOL                 0x00000200
#define AID_PARAMS_TDOL                 0x00000400
#define AID_PARAMS_PCD_TRANS_LIMIT      0x00000800
#define AID_PARAMS_CVM_REQ_LIMIT        0x00001000
#define AID_PARAMS_AMOUNT_ZERO_ONLINE   0x00002000
#define AID_PARAMS_STATUS_CHECK         0x00004000
#define AID_PARAMS_CURRENCY_EXP         0x00008000
#define AID_PARAMS_MS_APP_VERSION       0x00010000
#define AID_PARAMS_TERM_CAP_NO_CVM      0x00020000
#define AID_PARAMS_ADD_TERM_CAP         0x00040000
#define AID_PARAMS_MERCH_CATEGORY       0x00080000
#define AID_PARAMS_UDOL                 0x00100000
#define AID_PARAMS_MERCH_CUSTOM_DATA    0x00200000
#define AID_PARAMS_APP_SELECT_IND       0x00400000
#define AID_PARAMS_SUBSCHEMES           0x00800000

/* indexed parameters stored per AID in the reader */
typedef struct {
	DWORD properties;
	AID aid;
	BYTE trans_currency_code[2];
	BYTE terminal_type;
	BYTE app_version[2];
	BYTE term_floor_limit[6];
	BYTE term_capabilities[3];
	BYTE target_percentage;
	BYTE max_target_percentage;
	BYTE threshold_random[6];
	BYTE tac_default[5];
	BYTE tac_denial[5];
	BYTE tac_online[5];
	BYTE ddol_len;
	BYTE ddol[DOL_MAXLGTH];
	BYTE tdol_len;
	BYTE tdol[DOL_MAXLGTH];
	BYTE pcd_trans_limit[6];
	BYTE cvm_req_limit[6];
	BOOLEAN amount_zero_online;
	BOOLEAN status_check;
	BYTE trans_currency_code_exp;
	BYTE ms_app_version[2];
	BYTE term_capabilities_no_cvm[3];
	BYTE add_term_capabilities[5];
	BYTE merchant_category_code[2];
	BYTE udol_len;
	BYTE udol[DOL_MAXLGTH];
	BYTE merchant_custom_data[20];
	BOOLEAN app_selection_indicator;
	BYTE subschemes;
} AID_PARAMS;

/* Specific parameters for EMV transactions */
typedef struct {
	BYTE country_code[2];
	BYTE trans_category_code;
	BOOLEAN fdda_v0_enable;         // visa only
	BYTE term_comm_check;
	BOOLEAN visa_wave_enable;
	BOOLEAN visa_wave_offline_enable;
	BOOLEAN visa_format_track1_enable;
	BOOLEAN visa_format_track2_enable;
	BYTE visa_crypto_subscheme;
	BYTE merchant_type_indicator;
	BYTE term_trans_info[3];
	BYTE term_option_status[2];
	DWORD enabled_apps;
	TIMEOUTS timeouts[TIMEOUT_LIST_LENGTH];
	DWORD emv_result_tags1;
	DWORD emv_result_tags2;
	DWORD ms_result_tags1;
	DWORD ms_result_tags2;
	AID_PARAMS aid_params[AID_LIST_LENGTH];
} EMV_PARAMS_STRUCT;

/* CA Private Key representation */
typedef struct {
	BYTE rid[5];
	BYTE index;
	BYTE hash_algo_indic;
	BYTE key_algo_indic;
	BYTE mod_len;
	BYTE mod[248];
	BYTE exp_len;
	BYTE exp[4];
	BYTE hash_result[20];
} OTI_CAPK_STRUCT;

#define EMV_TRANS_TYPE  0x01
#define EMV_TRANS_TIME  0x02
#define EMV_TRANS_DUKPT 0x04
//TODO: add SmarTap Start Mode

/* Parameters for PollEMV call */
typedef struct {
	BYTE properties;		//!< bitfield to indicate presence of optional fields
	BYTE amt_auth[6];		//!< amount
	BYTE date[3];			//!< date
	BYTE trans_type;		//!< transaction type (optional)
	BYTE trans_time[3];		//!< transaction time (optional)
	BYTE currency_code[2];	//!< currency code
	BYTE dukpt_key[1];
} ACT_EMV_STRUCT;

/* List of properties set inside the Resp struct */
#define RESP1_TRACK1                    0x00000001
#define RESP1_TRACK2                    0x00000002
#define RESP1_CVM_LIST                  0x00000004
#define RESP1_CVM_RESULTS               0x00000008
#define RESP1_UNPRED_NUM                0x00000010
#define RESP1_TVR                       0x00000020
#define RESP1_TRANS_DATE                0x00000040
#define RESP1_CID                       0x00000080
#define RESP1_TSI                       0x00000100
#define RESP1_CAPK_INDEX                0x00000200
#define RESP1_APP_CRYPT                 0x00000400
#define RESP1_ISS_APPDATA               0x00000800
#define RESP1_DF_NAME                   0x00001000
#define RESP1_ATC                       0x00002000
#define RESP1_TRX_RESULT                0x00004000
#define RESP1_DATA_AUTH_CODE            0x00008000
#define RESP1_AIP                       0x00010000
#define RESP1_TTQ                       0x00020000
#define RESP1_PAN                       0x00040000
#define RESP1_PAN_SEQ                   0x00080000
#define RESP1_EXP_DATE                  0x00100000
#define RESP1_APP_LABEL                 0x00200000
#define RESP1_ICC_DYN_NUM               0x00400000
#define RESP1_HOLDER_NAME               0x00800000
#define RESP1_AUC                       0x01000000
#define RESP1_IAC                       0x02000000
#define RESP1_AUTH_CODE                 0x04000000
#define RESP1_OFF_AMOUNT                0x08000000
#define RESP1_AID                       0x10000000
#define RESP1_PICC_TYPE                 0x20000000
#define RESP1_POLL_EMV_ERROR            0x40000000
#define RESP1_AMOUNT_AUTHORIZED         0x80000000

#define RESP2_TRANS_TYPE                0x00000001
#define RESP2_DATA_CHANNEL              0x00000002
#define RESP2_BANK_ID_CODE              0x00000004
#define RESP2_APP_DISC_DATA             0x00000008
#define RESP2_TRANS_SEQ_CNTR            0x00000010
#define RESP2_HOLDER_NAME_EXT           0x00000020
#define RESP2_FORM_FACTOR_IND           0x00000040
#define RESP2_PP_3RD_PARTY_DATA         0x00000080
#define RESP2_CUST_EXCL_DATA            0x00000100
#define RESP2_APP_EFFECTIVE_DATE        0x00000200
#define RESP2_CTQ                       0x00000400
#define RESP2_MS_APP_VER_NUM            0x00000800
#define RESP2_PCD_SERIAL_NUM            0x00001000
#define RESP2_TERM_CAPABILITIES         0x00002000
#define RESP2_TERM_CAPABILITIES_NO_CVM  0x00004000
#define RESP2_COUNTRY_CODE              0x00008000
#define RESP2_TERMINAL_TYPE             0x00010000
#define RESP2_TRACK2_EQ_DATA            0x00020000
#define RESP2_TRANS_CATEGORY_CODE       0x00040000
#define RESP2_TRANS_CURRENCY_CODE       0x00080000

/* Response data to EMV Polling */
typedef struct {
	DWORD properties1;
	DWORD properties2;
	BYTE track1_len;
	BYTE track1_data[79];
	BYTE track2_len;
	BYTE track2_data[40];
	WORD cvm_list_len;
	BYTE cvm_list[252];
	BYTE cvm_results[3];
	BYTE unpred_num[4];
	BYTE tvr[5];
	BYTE date[3];
	BYTE cid;
	BYTE tsi[2];
	BYTE capk_index;
	BYTE app_crypt[8];
	BYTE iss_app_data_len;
	BYTE iss_app_data[32];
	BYTE df_name_len;
	BYTE df_name[16];
	BYTE atc[2];
	BYTE pcd_trans_result;
	BYTE data_auth_code[2];
	BYTE aip[2];
	BYTE ttq[4];
	BYTE pan_len;
	BYTE pan[10];
	BYTE pan_seq_num;
	BYTE exp_date[3];
	BYTE app_label_len;
	BYTE app_label[16];
	BYTE icc_dyn_num_len;
	BYTE icc_dyn_num[8];
	BYTE cardholder_name_len;
	BYTE cardholder_name[26];
	BYTE auc[2];
	BYTE iac_default[5];
	BYTE iac_denial[5];
	BYTE iac_online[5];
	BYTE auth_code[6];
	BYTE offline_spending_amount[6];
	BYTE aid_len;
	BYTE aid[AID_MAXLGTH];
	BYTE picc_type;
	BYTE poll_emv_error;
	BYTE amount_authorized[6];

	BYTE trans_type;
	BYTE data_channel;
	BYTE bank_id_code_len;
	BYTE bank_id_code[11];
	BYTE app_disc_data_len;
	BYTE app_disc_data[32];
	BYTE trans_seq_counter_len;
	BYTE trans_seq_counter[4];
	BYTE holder_name_ext_len;
	BYTE holder_name_ext[45];
	BYTE form_factor_ind[4];
	BYTE paypass_3rd_party_data_len;
	BYTE paypass_3rd_party_data[32];
	BYTE customer_exclusive_data_len;
	BYTE customer_exclusive_data[32];
	BYTE app_effective_date[3];
	BYTE ctq[2];
	BYTE ms_app_ver_num[2];
	BYTE pcd_serial_num[8];
	BYTE term_capabilities[3];
	BYTE term_capabilities_no_cvm[3];
	BYTE country_code[2];
	BYTE terminal_type;
	BYTE track2_eq_data_len;
	BYTE track2_eq_data[19];
	BYTE trans_category_code;
	BYTE trans_currency_code[2];
} EMV_RESP_STRUCT;

/* Possible online status */
typedef enum {
	OTI_STAT_FAILED = 0, //!< Displays an online declination message
	OTI_STAT_SUCCEEDED,  //!< Displays an online approval message
	OTI_STAT_WAIT		 //!< Restarts its timer and wait for another OnlineStatus message
} ONLINE_STATUS;

/* Potentially supported applications */
#define APPLI_MC_PAYPASS        0x01000000
#define APPLI_MAESTRO_PAYPASS   0x02000000
#define APPLI_AMEX              0x04000000
#define APPLI_VISA              0x08000000
#define APPLI_ISO_14443         0x10000000
#define APPLI_MIFARE            0x20000000
#define APPLI_MC_VIRTUAL        0x40000000
#define APPLI_VISA_VIRTUAL      0x80000000
#define APPLI_VISA_ELECTRON     0x00010000
#define APPLI_DISCOVER          0x00020000
#define APPLI_VISA_VPAY         0x00080000
#define APPLI_MC_DEBIT          0x00100000
#define APPLI_BP                0x00200000
#define APPLI_VISA_TEST         0x00400000
#define APPLI_VINCINITY         0x00800000
#define APPLI_BP_TEST           0x00000400
#define APPLI_INTERAC_FLASH     0x00000800
#define APPLI_ISIS_SMARTAP      0x00002000
#define APPLI_GOOGLE_WALLET     0x00004000

#define PAYPASS_MC_AID          "\xA0\x00\x00\x00\x04\x10\x10"
#define PAYPASS_MC_AID_LEN      7
#define PAYPASS_MAESTRO_AID     "\xA0\x00\x00\x00\x04\x30\x60"
#define PAYPASS_MAESTRO_AID_LEN 7
#define MC_DEBIT_AID            "\xA0\x00\x00\x00\x04\x88\x26"
#define MC_DEBIT_AID_LEN        7
#define PAYPASS_VIRTUAL_AID     "\xB0\x12\x34\x56\x78"
#define PAYPASS_VIRTUAL_AID_LEN 5
#define VISA_AID                "\xA0\x00\x00\x00\x03\x10\x10"
#define VISA_AID_LEN            7
#define VISA_ELECTRON_AID       "\xA0\x00\x00\x00\x03\x20\x10"
#define VISA_ELECTRON_AID_LEN   7
#define VISA_VPAY_AID           "\xA0\x00\x00\x00\x03\x20\x20"
#define VISA_VPAY_AID_LEN       7
#define VISA_TEST_AID           "\xA0\x00\x00\x99\x99\x01"
#define VISA_TEST_AID_LEN       6
#define VISA_VIRTUAL_AID        "\xA0\x00\x00\x00\x99\x90\x90"
#define VISA_VIRTUAL_AID_LEN    7
#define EXPRESS_PAY_AID         "\xA0\x00\x00\x00\x25\x01"
#define EXPRESS_PAY_AID_LEN     6
#define DISCOVER_AID            "\xA0\x00\x00\x03\x24\x10\x10"
#define DISCOVER_AID_LEN        7
#define INTERAC_FLASH_AID       "\xA0\x00\x00\x02\x77\x10\x10"
#define INTERAC_FLASH_AID_LEN   7
#define ISIS_SMARTAP_AID        "\xA0\x00\x00\x04\x85\x10\x01\x01\x01"
#define ISIS_SMARTAP_AID_LEN    9

/* Sub-schemes */
#define SUBSCHEME_MAGSTRIPE     0x01
#define SUBSCHEME_EMV           0x02

/* User Interface mode */
typedef enum {
	MASTERCARD_UI = 0,	//!< PCD behaves according to MasterCard "Terminal Implementation Guide"
	EUROPE_UI = 1		//!< PCD behaves according to "Contactless terminal User Interface For Europe & UK" V1.7 (Default settings)
} UI_MODE;

/* External Display Settings */
typedef enum {
	NO_EXT_DISPLAY = 0,		//!< No external display (Default setting)
	EXT_MSG_INDEX_ONLY = 1,	//!< Send only Preset Message Index 
	EXT_FILTERED_MSG = 2,	//!< Send "Filtered" Preset Messages: only CR and LF between ASCII lines.
	EXT_FULL_MSG = 3		//!< Send Preset Messages
} EXT_DISPLAY_MODE;

/** @brief LED patterns available */
typedef enum
{
	SUCCESS_PATTERN_UI1 = 0x00, //!< Old  OTI UI
	SUCCESS_PATTERN_UI2,		//!< Success according to "Contactless terminal User Interface For Europe and UK, Version 1.7"
	FAILED_PATTERN_UI2,			//!< Fail according to "Contactless terminal User Interface For Europe and UK, Version 1.7"
	ALL_LEDS_ON_PATTERN,
	ALL_LEDS_OFF_PATTERN,
	LED1_ON_PATTERN,
	LED1_BLINK_PATTERN,
	TERMINATE_WITH_LED1_PATTERN,
	LED1_ON_OTHER_LEDS_CHASING,
} UI_PATTERN;


#endif


/* Doxygen format:*/
/**
* \defgroup API Library APIs
* The following sections describe the OTI Library API.
* The APIs are divided to groups based on their functionality.
*  @{
*	\defgroup UI User Interface APIs
*		APIs to control the Readers Display, LEDs and Buzzer.
*		@{ \defgroup LCD Graphic Display functions
*		@}
*	\defgroup CFG Configuration APIs
*	Control Reader settings and Keys management APIs.
*		The reader holds 2 copies of the configuration parameters:
*		- "Runtime" values are stored in RAM. 
*		- "Default" values are stored in EEPROM. 
* 
*		Saturn Host-PCD protocol allows to configure the different settings in 2 ways:
*		- The DO command set the parameter value in RAM but does not change it's value in the EEPROM, 
*			so calling the LoadDefaultParams() or ResetPCD() will restore the default values and overwrite the "DO" values. 
*		- The SET command changes the value in the EEPROM but does not change it's value in the RAM until LoadDefaultParams() 
*			is called or until the reader is reset.
*		@{ \defgroup KEY Key management
*		@}
*	\defgroup EMV EMV Transaction APIs
*	Payment transaction APIs
*	\defgroup GENERAL General APIs
*			Application flow and firmware update APIs
*  @}
* \defgroup INTERNAL Internal library functions
* 
*		@{ \defgroup TLV TLV utility functions
*		@}
* \defgroup DEFINES Define Sets
*/

