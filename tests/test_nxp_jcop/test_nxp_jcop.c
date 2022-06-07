#include <stdio.h>
#include "oti_types.h"
#include "tlv.h"
#include "otilib.h"
#include "frames.h"
#include "Poll.h"

void setCommandBuf( MSG_BUFF* databuff, BYTE* cmd, int len ) {
    databuff->Data = cmd;
    databuff->Max_Length = len;
    databuff->Actual_Length = len;
}

int main(int argc, char **argv)
{
    BYTE msg[900];
    MSG_BUFF msgBuf;
    msgBuf.Data = msg;
    msgBuf.Max_Length = 900;

    OTI_STATUS ret;
    // SELECT applet cmd: 00A4 0400 09 4D7954657374417070 00
    BYTE cmdSelectApplet[] = { 0x00, 0xA4, 0x04, 0x00, 0x09, 0x4D, 0x79, 0x54,
                               0x65, 0x73, 0x74, 0x41, 0x70, 0x70, 0x00 };

    // CMD_GET_CARD_UID = (short) 0x0045;
    BYTE cmdGetCardUid[] = { 0x00, 0x45, 0x00, 0x00, 0x00 };

    // CMD_KM_GET_PUBLIC_KEY = (short) 0x0022;
    BYTE cmdGetPublicKey[] = { 0x00, 0x22, 0x00, 0x00, 0x00 };

    // CMD_GET_NONCE = (short) 0x0040
    BYTE cmdGetNonce[] = { 0x00, 0x40, 0x00, 0x00, 0x00 };

    MSG_BUFF databuff;
    setCommandBuf( &databuff, cmdSelectApplet, sizeof(cmdSelectApplet) );

    BYTE buf[512];
    MSG_BUFF respBuff;
    respBuff.Data = buf;
    respBuff.Max_Length = 512;

    POLL_MODE mode = ACTIVATE_ONCE;

    BYTE track1[50];
    BYTE track2[50];
    POLL_RESP_STRUCT resp;
    resp.track1_data = track1;
    resp.track1_len = 50;
    resp.track2_data = track2;
    resp.track2_len = 50;

    if(argc != 2)
    {
        LOG_OUTPUT("Error! Invalid arguments.\r\n");
        LOG_OUTPUT("Using example: %s COM_PORT\r\n", argv[0]);
        LOG_OUTPUT("press ENTER to exit...");
        getchar();
        return 1;
    }

    if (SetReaderPort((BYTE *)argv[1]) != OTI_OK)
    {
        LOG_OUTPUT("Opening reader comm failed!");
        LOG_OUTPUT("press ENTER to exit...");
        getchar();
        return 2;
    }

    SetDebug(FALSE);    // Set to TRUE to see communication trace
    
    LOG_OUTPUT("\r\nPresent card\r\n");

    /* Example to retrieve a card's AID in transparent mode */
    BYTE enable_app_cmd[] = { 0xDf, 0x4F, 0x04, 0x10, 0x00, 0x00, 0x00 };
    RESP_MSG_STRUCT msg_resp;
    WORD bufLen = sizeof(msg);
    ExchangeCmd(OP_DO, enable_app_cmd, sizeof(enable_app_cmd), msg, &bufLen, &msg_resp, 1000);
    Poll(OP_DO, &mode, &resp, &msgBuf);

    /* Send SELECT */
    ret = TransparentData(&databuff, &respBuff, &msgBuf);
    if (ret != OTI_OK)
    {
        LOG_OUTPUT("\r\n\r\nError on Transparent NXP Data:%d\r\n",ret);
        LOG_OUTPUT("press ENTER to exit...");
        getchar();
        return ret;
    }
    /* FCI is in buf, check whether it's final response */
    if ((respBuff.Data[respBuff.Actual_Length - 2] == 0x90) && (respBuff.Data[respBuff.Actual_Length - 1] == 0x00))
    {
        LOG_OUTPUT("Card FCI: ");
        PrintBuf(respBuff.Data, (WORD)(respBuff.Actual_Length - 2));
        LOG_OUTPUT("\r\n");

        setCommandBuf( &databuff, cmdGetCardUid, sizeof( cmdGetCardUid ) );

        ret = TransparentData(&databuff, &respBuff, &msgBuf);

        if ((respBuff.Data[respBuff.Actual_Length - 2] == 0x90) && (respBuff.Data[respBuff.Actual_Length - 1] == 0x00)) {
            LOG_OUTPUT("Card Uid: \r\n");
            PrintBuf(respBuff.Data, (WORD)(respBuff.Actual_Length - 2));
            LOG_OUTPUT("\r\n");
        }

        setCommandBuf( &databuff, cmdGetPublicKey, sizeof( cmdGetPublicKey ) );

        ret = TransparentData(&databuff, &respBuff, &msgBuf);

        if ((respBuff.Data[respBuff.Actual_Length - 2] == 0x90) && (respBuff.Data[respBuff.Actual_Length - 1] == 0x00)) {
            LOG_OUTPUT("Card Public Key : \r\n");
            PrintBuf(respBuff.Data, (WORD)(respBuff.Actual_Length - 2));
            LOG_OUTPUT("\r\n");
        }

        setCommandBuf( &databuff, cmdGetNonce, sizeof( cmdGetNonce ) );

        ret = TransparentData(&databuff, &respBuff, &msgBuf);

        if ((respBuff.Data[respBuff.Actual_Length - 2] == 0x90) && (respBuff.Data[respBuff.Actual_Length - 1] == 0x00)) {
            LOG_OUTPUT("Nonce : \r\n");
            PrintBuf(respBuff.Data, (WORD)(respBuff.Actual_Length - 2));
            LOG_OUTPUT("\r\n");
        }
    }

    LOG_OUTPUT("press ENTER to exit...");
    fflush(stdin);
    getchar();

    return OTI_OK;
}
