#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

#include "types.h"

#include "UdpLib/myHaloUdp.h"
#include "UdpLib/halo_udp_comm.h"
#include "UdpLib/halo_udp_stats.h"
#include "UdpLib/crc16.h"

#include "halo_udp_tests.h"
#include "test_msg_queue.h"
#include "test_control_interface.h"

//Work functions
extern int myHalo_udp_state_changed(void);
extern int getNextTxSeqNum();
extern void reset_test_state();

//txGoodTest Callbacks

static int pktsGenerated = 0;
static int rxPktsGenerated = 0;
#define SEQ_NUM_RANGE  65536
#define TARGET_GENERATED_PKTS  (SEQ_NUM_RANGE + 100)

uint8 haloUdpRxMsg[] = {0x00,0x02,0x01,0x00,0x00,0x00,0x30,0x00,0x01,0x00,0x06,
                        0x00,0xf0,0x06,0x00,0x00,0xb2,0xb8,0x21,0x4f,0x64,0x00,
                        0x00,0x00,0xd4,0xfe,0xff,0xff,0xf4,0x01,0x00,0x00,0x00,
                        0x00,0x00,0x00,0x57,0x01,0x00,0x00,0x00,0x00,0x2c,0x60,
                        0xb8,0x0b,0x00,0x00,0xb2,0xb8,0x21,0x4f,0x01,0x07,0x00,
                        0x00,0x81,0x02
                       };
int haloUdpRxMsgLength = sizeof(haloUdpRxMsg);

void sendPkts(int numPkts)
{
    uint8 msg[] = {0x01,0x00,0x06,
                   0x00,0xf0,0x06,0x00,0x00,0xb2,0xb8,0x21,0x4f,0x64,0x00,
                   0x00,0x00,0xd4,0xfe,0x17,0xff,0xf4,0x01,0x00,0x00,0x00,
                   0x00,0x00,0x00,0x57,0x01,0x00,0x00,0x00,0x00,0x2c,0x60,
                   0xb8,0x0b,0x00,0x00,0xb2,0xb8,0x21,0x4f,0x01,0x07,0x00,
                   0x00
                  };
    int msgLength = sizeof(msg);
    GenericIP socketAddress = GENERIC_IP_INIT();
    int i;

    for (i = 0; i < numPkts; i++)
    {
        //Send a message
        halo_msg_sendto((HaloMessage *) msg, socketAddress);
        pktsGenerated++;
        //printf("pktsGenerated: %d\n", pktsGenerated);
    }
}

int txSeqNumBeyondSeqNumRange_SentCallback(void *args)
{
    int passed = 0;
    HaloUdpEventData *sentEventData = (HaloUdpEventData *) args;

    if (pktsGenerated < TARGET_GENERATED_PKTS)
    {
        //printf("# pkts generated: %d\n", pktsGenerated);
        sendPkts(1);
    }

    passed = 1;

    return passed;
}

int txSeqNumBeyondSeqNumRange(void *args)
{
    HaloUdpStats stats = HALO_UDP_STATS_INIT();
    int stop = 0;
    int timeout = 360; //in seconds
    int passed = 0;

    pktsGenerated = 0;

    reset_test_state();
    setTestSentCallback(txSeqNumBeyondSeqNumRange_SentCallback);

    enableAckGenToRxData();
    enableSessionResponseData();

    sendPkts(1000);

    do
    {
        if (myHalo_udp_state_changed())
        {
            stats = get_halo_msg_stats();

            if ((stats.txPkts == TARGET_GENERATED_PKTS)&&(stats.txDataPkts == TARGET_GENERATED_PKTS)
                    &&(stats.txConfirmedPkts == TARGET_GENERATED_PKTS)&&(stats.txConfirmedBytes > 0)
                    &&(stats.txDroppedPkts == 0)&&(stats.txDataBytes > 0)
                    &&(stats.txBytes > 0)&&(stats.rxAcks == TARGET_GENERATED_PKTS)
                    &&(!wasDroppedFuncCalled())&&(wasSentFuncCalled())
                    &&(getSentFuncCalled() == TARGET_GENERATED_PKTS))
            {
                //Confirmed there is a stop
                stop = 1;
                passed = 1;
            }
        }

        sleep(1);

        //report progress
        printf("txSeqNumBeyondSeqNumRange: %d out of %d (%3d%% done)\n", stats.txConfirmedPkts, TARGET_GENERATED_PKTS, ((stats.txConfirmedPkts*100)/TARGET_GENERATED_PKTS));

        timeout--;
    }
    while ((!stop)&&(timeout));

    if (!timeout)
    {
        printf("Test Timed Out!\n");
    }

    printf("Tx Beyond SeqNum Range: %s\n", passed ? "PASSED" : "FAILED");

    return passed;
}

void receivePkts(int numPkts, int new_session)
{
    MyHaloUdpHeader *header = NULL;
    int i;
    uint16 crc;

    for (i = 0; i < numPkts; i++)
    {
        //Copy over the data, assign sequence number, then recalculate crc for udp processing
        header = (MyHaloUdpHeader *) &haloUdpRxMsg[0];

        if(new_session)
        {
            header->status |= NEW_SESSION;
        }
        else
        {
            header->status &= ~NEW_SESSION;
        }

        header->seqNum = getNextTxSeqNum();
        header->payloadLength = (haloUdpRxMsgLength - sizeof(MyHaloUdpHeader) - sizeof(uint16));
        crc = hdlcFcs16(hdlc_init_fcs16, (const uint8 *) &haloUdpRxMsg[0], haloUdpRxMsgLength - sizeof(uint16));
        memcpy(&haloUdpRxMsg[haloUdpRxMsgLength - sizeof(uint16)], &crc, sizeof(uint16));
        enqueueMsg(&haloUdpRxMsg[0], haloUdpRxMsgLength);
        rxPktsGenerated++;
    }
}

int rxSeqNumBeyondSeqNumRange_RcvdCallback(void *args)
{
    int passed = 0;
    HaloUdpEventData *rcvdEventData = (HaloUdpEventData *) args;

    if (rxPktsGenerated < TARGET_GENERATED_PKTS)
    {
        //printf("# pkts generated: %d\n", rxPktsGenerated);
        receivePkts(1, 0);
    }

    passed = 1;

    return passed;
}

int rxSeqNumBeyondSeqNumRange(void *args)
{
    HaloUdpStats stats = HALO_UDP_STATS_INIT();
    int stop = 0;
    int timeout = 360;//240; //in seconds
    int passed = 0;

    reset_test_state();
    setTestRcvdCallback(rxSeqNumBeyondSeqNumRange_RcvdCallback);
    halo_msg_new_session(0); //Create a new session to clear any previous seq history
    rxPktsGenerated = 0;

    //NOTE: If the memory needed for this is too much new msgs could be generated as rx finds them
    //receivePkts(TARGET_GENERATED_PKTS, 0);
    receivePkts(1, 0);

    do
    {
        if (myHalo_udp_state_changed())
        {
            stats = get_halo_msg_stats();

            if ((stats.rxGoodPkts == TARGET_GENERATED_PKTS)
                    &&(stats.rxGoodBytes == (TARGET_GENERATED_PKTS*haloUdpRxMsgLength))
                    &&(stats.rxDataPkts == TARGET_GENERATED_PKTS)
                    &&(getRcvFuncCalled() == TARGET_GENERATED_PKTS))
            {
                //Confirmed there is a stop
                stop = 1;
                passed = 1;
            }
        }

        //report progress
        printf("rxSeqNumBeyondSeqNumRange: %d out of %d (%3d%% done)\n", stats.rxDataPkts, TARGET_GENERATED_PKTS, ((stats.rxDataPkts*100)/TARGET_GENERATED_PKTS));

        sleep(1);
        timeout--;
    }
    while ((!stop)&&(timeout));

    if (!timeout)
    {
        printf("Test Timed Out!\n");
    }

    printf("Rx Beyond SeqNum Range: %s\n", passed ? "PASSED" : "FAILED");

    return passed;
}


static int txSessionControl_NewSessionFlagSet = 0;
int txSessionControlTest_SendCallback(void *args)
{
    int passed = 0;
    HaloUdpEventData *rcvdEventData = (HaloUdpEventData *) args;
    MyHaloUdpHeader *header = NULL;

    //Copy over the data, assign sequence number, then recalculate crc for udp processing
    header = (MyHaloUdpHeader *) rcvdEventData->data;

    if (header->status & NEW_SESSION) //New session flag present
    {
        txSessionControl_NewSessionFlagSet++;
    }

    passed = 1;

    return passed;
}

int txSessionControlTest(void *args)
{
    uint8 msg[] = {0x01,0x00,0x06,
                   0x00,0xf0,0x06,0x00,0x00,0xb2,0xb8,0x21,0x4f,0x64,0x00,
                   0x00,0x00,0xd4,0xfe,0x17,0xff,0xf4,0x01,0x00,0x00,0x00,
                   0x00,0x00,0x00,0x57,0x01,0x00,0x00,0x00,0x00,0x2c,0x60,
                   0xb8,0x0b,0x00,0x00,0xb2,0xb8,0x21,0x4f,0x01,0x07,0x00,
                   0x00
                  };
    int msgLength = sizeof(msg);
    HaloUdpStats stats = HALO_UDP_STATS_INIT();
    int stop = 0;
    int timeout = 80; //in seconds
    int passed = 0;
    GenericIP socketAddress = GENERIC_IP_INIT();
    int stage = 0;

    reset_test_state();

    halo_msg_new_session(0); //Create a new session to clear any previous seq history
    txSessionControl_NewSessionFlagSet = 0;
    setTestSendCallback(txSessionControlTest_SendCallback);

    //Send a message
    halo_msg_sendto((HaloMessage *) msg, socketAddress);

    do
    {
        if (myHalo_udp_state_changed())
        {
            stats = get_halo_msg_stats();

            if (stage == 0)
            {
                //Confirm that new session flag set for all tx retries

                if ((stats.txPkts == MAX_REXMIT)&&(stats.txDataPkts == MAX_REXMIT)
                        &&(stats.txConfirmedPkts == 0)&&(stats.txConfirmedBytes == 0)
                        &&(stats.txDroppedPkts == 1)&&(stats.txDataBytes > 0)
                        &&(stats.txBytes > 0)&&(stats.rxAcks == 0)
                        &&(txSessionControl_NewSessionFlagSet == MAX_REXMIT)
                        &&(wasDroppedFuncCalled())&&(!wasSentFuncCalled()))
                {
                    txSessionControl_NewSessionFlagSet = 0;
                    halo_msg_reset_stats();
                    setDroppedFuncCalled(0);

                    enableAckGenToRxData();
                    enableSessionResponseData();

                    //Send a message
                    halo_msg_sendto((HaloMessage *) msg, socketAddress);

                    stage = 1;
                }
            }
            else if (stage == 1)
            {
                //Confirm that new session flag stays set after a tx drop

                if ((stats.txPkts == 1)&&(stats.txDataPkts == 1)
                        &&(stats.txConfirmedPkts == 1)&&(stats.txConfirmedBytes > 0)
                        &&(stats.txDroppedPkts == 0)&&(stats.txDataBytes > 0)
                        &&(stats.txBytes > 0)&&(stats.rxAcks == 1)
                        &&(txSessionControl_NewSessionFlagSet == 1)
                        &&(!wasDroppedFuncCalled())&&(wasSentFuncCalled()))
                {
                    txSessionControl_NewSessionFlagSet = 0;
                    halo_msg_reset_stats();
                    setSentFuncCalled(0);

                    enableAckGenToRxData();
                    enableSessionResponseData();

                    //Send a message
                    halo_msg_sendto((HaloMessage *) msg, socketAddress);

                    stage = 2;
                }
            }
            else if (stage == 2)
            {
                //Confirm that new session flag is no longer set after a successful send/a session_restart is sent

                if ((stats.txPkts == 1)&&(stats.txDataPkts == 1)
                        &&(stats.txConfirmedPkts == 1)&&(stats.txConfirmedBytes > 0)
                        &&(stats.txDroppedPkts == 0)&&(stats.txDataBytes > 0)
                        &&(stats.txBytes > 0)&&(stats.rxAcks == 1)
                        &&(!txSessionControl_NewSessionFlagSet)
                        &&(!wasDroppedFuncCalled())&&(wasSentFuncCalled()))
                {
                    //Confirmed there is a stop
                    stop = 1;
                    passed = 1;
                }
            }
        }

        sleep(1);
        timeout--;
    }
    while ((!stop)&&(timeout));

    if (!timeout)
    {
        printf("Test Timed Out!\n");
    }

    printf("Tx Session Control Test: %s\n", passed ? "PASSED" : "FAILED");

    return passed;
}


static int rxSessionControl_NewSessionConfirmed = 0;

int rxSessionControlTest_SendCallback(void *args)
{
    int passed = 0;
    HaloUdpEventData *rcvdEventData = (HaloUdpEventData *) args;
    MyHaloUdpHeader *header = NULL;

    //Copy over the data, assign sequence number, then recalculate crc for udp processing
    header = (MyHaloUdpHeader *) rcvdEventData->data;

    if (header->status & SESSION_RESTARTED) //New session flag present
    {
        rxSessionControl_NewSessionConfirmed++;
    }

    passed = 1;

    return passed;
}

int rxSessionControlTest(void *args)
{
    HaloUdpStats stats = HALO_UDP_STATS_INIT();
    int stop = 0;
    int timeout = 40; //40 second timeout
    int passed = 0;
    GenericIP socketAddress = GENERIC_IP_INIT();

    reset_test_state();

    rxSessionControl_NewSessionConfirmed = 0;
    setTestSendCallback(rxSessionControlTest_SendCallback);

    //Generate one new session packet
    receivePkts(1, 1);

    do
    {
        if (myHalo_udp_state_changed())
        {
            stats = get_halo_msg_stats();

            if ((stats.rxGoodPkts == 1)
                &&(stats.rxGoodBytes == haloUdpRxMsgLength)
                &&(stats.rxDataPkts == 1)
                &&(stats.rxDataBytes == haloUdpRxMsgLength)
                &&(rxSessionControl_NewSessionConfirmed)
                &&(wasRcvFuncCalled()))
            {
                //Confirmed there is a stop
                stop = 1;
                passed = 1;
            }

        }

        sleep(1);
        timeout--;
    }
    while ((!stop)&&(timeout));

    if (!timeout)
    {
        printf("Test Timed Out!\n");
    }

    printf("Rx Session Control Test: %s\n", passed ? "PASSED" : "FAILED");

    return passed;
}

void receiveInvalidPayloadPkts(int numPkts)
{
    MyHaloUdpHeader *header = NULL;
    int i;
    uint16 crc;

    for (i = 0; i < numPkts; i++)
    {
        //Copy over the data, assign sequence number, then recalculate crc for udp processing
        header = (MyHaloUdpHeader *) &haloUdpRxMsg[0];
        header->seqNum = getNextTxSeqNum();

        header->payloadLength = 60000; //pkt should be smaller than 60000 bytes

        crc = hdlcFcs16(hdlc_init_fcs16, (const uint8 *) &haloUdpRxMsg[0], haloUdpRxMsgLength - sizeof(uint16));
        memcpy(&haloUdpRxMsg[haloUdpRxMsgLength - sizeof(uint16)], &crc, sizeof(uint16));
        enqueueMsg(&haloUdpRxMsg[0], haloUdpRxMsgLength);
    }
}

int rxInvalidPayloadLengthTest(void *args)
{
    HaloUdpStats stats = HALO_UDP_STATS_INIT();
    int stop = 0;
    int timeout = 40; //40 second timeout
    int passed = 0;
    GenericIP socketAddress = GENERIC_IP_INIT();

    reset_test_state();

    //Generate one new session packet
    receiveInvalidPayloadPkts(1);

    do
    {
        if (myHalo_udp_state_changed())
        {
            stats = get_halo_msg_stats();

            if ((stats.rxGoodPkts == 0)
                &&(stats.rxGoodBytes == 0)
                &&(stats.rxDataPkts == 0)
                &&(stats.rxDataBytes == 0)
                &&(stats.rxInvalidPayloadPkts == 1)
                &&(!wasRcvFuncCalled()))
            {
                //Confirmed there is a stop
                stop = 1;
                passed = 1;
            }

        }

        sleep(1);
        timeout--;
    }
    while ((!stop)&&(timeout));

    if (!timeout)
    {
        printf("Test Timed Out!\n");
    }

    printf("Rx Invalid Payload Length Test: %s\n", passed ? "PASSED" : "FAILED");

    return passed;
}
