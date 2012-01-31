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
int myHalo_udp_state_changed(void)
{
    static HaloUdpStats prevStats = HALO_UDP_STATS_INIT();
    int changed = 0;
    HaloUdpStats stats = get_halo_msg_stats();

    //Stats Change
    if ((prevStats.txBytes != stats.txBytes)||(prevStats.txConfirmedBytes != stats.txConfirmedBytes) ||
            (prevStats.txPkts != stats.txPkts)||(prevStats.txConfirmedPkts != stats.txConfirmedPkts)||
            (prevStats.txDroppedPkts != stats.txDroppedPkts)||(prevStats.txAcks != stats.txAcks)||
            (prevStats.txDataPkts != stats.txDataPkts)||(prevStats.txDataBytes != stats.txDataBytes)||
            (prevStats.rxGoodBytes != stats.rxGoodBytes)||(prevStats.rxGoodPkts != stats.rxGoodPkts)||
            (prevStats.rxBadCrcPkts != stats.rxBadCrcPkts)||(prevStats.rxDuplicatePkts != stats.rxDuplicatePkts)||
            (prevStats.rxRuntPkts != stats.rxRuntPkts)||(prevStats.rxInvalidPayloadPkts != stats.rxInvalidPayloadPkts)||
            (prevStats.rxAcks != stats.rxAcks)||(prevStats.rxDataPkts != stats.rxDataPkts)||
            (prevStats.rxDataBytes != stats.rxDataBytes))
    {
        changed = 1;

        //Update Prev stats
        prevStats = stats;
    }

    //printf("myHalo UDP Change detected!\n");

    return changed;
}

void reset_test_state()
{
    //Makes sure stats are clear
    halo_msg_reset_stats();

    //Clear callback status state
    setRcvFuncCalled(0);
    setSentFuncCalled(0);
    setDroppedFuncCalled(0);
    setTestRcvdCallback(NULL);
    setTestSentCallback(NULL);
    setTestDroppedCallback(NULL);

    setSendFuncCalled(0);
    setTestSendCallback(NULL);

    disableAckGenToRxData();
    disableSessionResponseData();

    clearMsgs();
}

//Actual Tests

int runtTest(void *args)
{
    uint8 rxBuf[] = {0x11,0x22,0x33};
    int rxLength = sizeof(rxBuf);
    HaloUdpStats stats = HALO_UDP_STATS_INIT();
    int stop = 0;
    int timeout = 10; //10 second timeout
    int passed = 0;

    reset_test_state();

    enqueueMsg(&rxBuf[0], rxLength);
    do
    {
        if (myHalo_udp_state_changed())
        {
            stats = get_halo_msg_stats();

            if ((stats.rxRuntPkts == 1)&&(stats.rxGoodPkts == 0)&&(!wasRcvFuncCalled()))
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

    printf("Rx Runt Test: %s\n", passed ? "PASSED" : "FAILED");

    return passed;
}

int goodPktTest(void *args)
{
    uint8 rxBuf[] = {0x00,0x02,0x01,0x00,0x00,0x00,0x30,0x00,0x01,0x00,0x06,
                     0x00,0xf0,0x06,0x00,0x00,0xb2,0xb8,0x21,0x4f,0x64,0x00,
                     0x00,0x00,0xd4,0xfe,0xff,0xff,0xf4,0x01,0x00,0x00,0x00,
                     0x00,0x00,0x00,0x57,0x01,0x00,0x00,0x00,0x00,0x2c,0x60,
                     0xb8,0x0b,0x00,0x00,0xb2,0xb8,0x21,0x4f,0x01,0x07,0x00,
                     0x00,0x81,0x02
                    };
    int rxLength = sizeof(rxBuf);
    HaloUdpStats stats = HALO_UDP_STATS_INIT();
    int stop = 0;
    int timeout = 10; //10 second timeout
    int passed = 0;

    reset_test_state();

    enqueueMsg(&rxBuf[0], rxLength);
    do
    {
        if (myHalo_udp_state_changed())
        {
            stats = get_halo_msg_stats();

            if ((stats.rxGoodPkts == 1)
                &&(stats.rxGoodBytes == rxLength)
                &&(stats.rxDataPkts == 1)
                &&(stats.rxDataBytes == rxLength)
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

    printf("Rx Good Pkt Test: %s\n", passed ? "PASSED" : "FAILED");

    return passed;
}

int duplicatePktTest(void *args)
{
    uint8 rxBuf[] = {0x01,0x02,0x07,0x00,0x00,0x00,0x30,0x00,0x07,0x00,0x06,
                     0x00,0x3d,0x01,0x00,0x00,0x2f,0xef,0x22,0x4f,0x12,0xfd,
                     0xff,0xff,0x84,0x00,0x00,0x00,0x13,0xfe,0xff,0xff,0xe7,
                     0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                     0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x00,
                     0x00,0xdb,0xfd
                    };
    int rxLength = sizeof(rxBuf);
    HaloUdpStats stats = HALO_UDP_STATS_INIT();
    int stop = 0;
    int timeout = 20;
    int passed = 0;
    int stage = 0;
    int duplicatesSent = 0;
    const int TARGET_DUPLICATES = 7;

    reset_test_state();

    enqueueMsg(&rxBuf[0], rxLength);
    do
    {
        if (myHalo_udp_state_changed())
        {
            stats = get_halo_msg_stats();

            if (stage == 0) //Convert valid packet gets sent
            {
                if ((stats.rxGoodPkts == 1)&&(stats.rxGoodBytes == rxLength)&&(wasRcvFuncCalled()))
                {
                    //Move to next stage

                    //Makes sure stats are clear
                    halo_msg_reset_stats();

                    //Clear callback status state
                    setRcvFuncCalled(0);
                    setSentFuncCalled(0);
                    setDroppedFuncCalled(0);

                    //Send First Duplicate
                    enqueueMsg(&rxBuf[0], rxLength);
                    duplicatesSent++;

                    stage = 1;
                }
            }
            else if (stage == 1)
            {
                if ((stats.rxGoodPkts == duplicatesSent)&&(stats.rxGoodBytes == rxLength*duplicatesSent)
                        &&(stats.rxDataPkts == 0)&&(stats.rxDataBytes == 0)
                        &&(stats.rxDuplicatePkts == duplicatesSent)&&(!wasRcvFuncCalled()))
                {
                    if (duplicatesSent == TARGET_DUPLICATES)
                    {
                        //Confirmed there is a stop
                        stop = 1;
                        passed = 1;
                    }
                    else
                    {
                        //Send another duplicate

                        enqueueMsg(&rxBuf[0], rxLength);
                        duplicatesSent++;
                    }
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

    printf("Rx Duplicate Pkt Test: %s\n", passed ? "PASSED" : "FAILED");

    return passed;
}

int rxOutOfSeqTest(void *args)
{
    #define SAMPLE_PKT_SIZE  58
    #define SEQ_NUM_ARRY_SIZE  8
    uint8 rxBuf[SAMPLE_PKT_SIZE] = {0x00,0x02,0x01,0x00,0x00,0x00,0x30,0x00,0x01,0x00,0x06,
                     0x00,0xf0,0x06,0x00,0x00,0xb2,0xb8,0x21,0x4f,0x64,0x00,
                     0x00,0x00,0xd4,0xfe,0xff,0xff,0xf4,0x01,0x00,0x00,0x00,
                     0x00,0x00,0x00,0x57,0x01,0x00,0x00,0x00,0x00,0x2c,0x60,
                     0xb8,0x0b,0x00,0x00,0xb2,0xb8,0x21,0x4f,0x01,0x07,0x00,
                     0x00,0x81,0x02
                    };
    int rxLength = sizeof(rxBuf);
    HaloUdpStats stats = HALO_UDP_STATS_INIT();
    int stop = 0;
    int timeout = 10; //10 second timeout
    int passed = 0;
    MyHaloUdpHeader *header = NULL;
    uint16 randSeqNums[SEQ_NUM_ARRY_SIZE] = {0x117, 0, 0xdead,0xffff,0xaaaa,0x7d3c,0x3994,0x457e};
    int numRandSeqEntries = (sizeof(randSeqNums) / sizeof(uint16));
    uint8 randSeqPkts[SEQ_NUM_ARRY_SIZE][SAMPLE_PKT_SIZE];
    int i = 0;
    uint16 crc;

    //Checks that static memory allocations were correct
    assert(rxLength == SAMPLE_PKT_SIZE);
    assert(numRandSeqEntries == SEQ_NUM_ARRY_SIZE);

    reset_test_state();

    halo_msg_new_session(0); //Create a new session to clear any previous seq history

    //Enter into the queue all of the out of sequence packets to simulate receipt
    for(i = 0; i < numRandSeqEntries; i++)
    {
        //Copy over the data, assign sequence number, then recalculate crc for udp processing
        memcpy(&randSeqPkts[i][0], rxBuf, SAMPLE_PKT_SIZE);
        header = (MyHaloUdpHeader *) &randSeqPkts[i][0];
        header->seqNum = randSeqNums[i];
        crc = hdlcFcs16(hdlc_init_fcs16, (const uint8 *) &randSeqPkts[i][0], SAMPLE_PKT_SIZE - sizeof(uint16));
        memcpy(&randSeqPkts[i][SAMPLE_PKT_SIZE - sizeof(uint16)], &crc, sizeof(uint16));
        enqueueMsg(&randSeqPkts[i][0], SAMPLE_PKT_SIZE);
    }

    do
    {
        if (myHalo_udp_state_changed())
        {
            stats = get_halo_msg_stats();

            if ((stats.rxGoodPkts == SEQ_NUM_ARRY_SIZE)
              &&(stats.rxGoodBytes == (SEQ_NUM_ARRY_SIZE*SAMPLE_PKT_SIZE))
              &&(stats.rxDataPkts == SEQ_NUM_ARRY_SIZE)
              &&(getRcvFuncCalled() == SEQ_NUM_ARRY_SIZE))
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

    printf("Rx Out of Sequence Test: %s\n", passed ? "PASSED" : "FAILED");

    return passed;
}

int badCrcTest(void *args)
{
    uint8 rxBuf[] = {0x00,0x02,0x01,0x00,0x00,0x00,0x30,0x00,0x01,0x00,0x06,
                     0x00,0xf0,0x06,0x00,0x00,0xb2,0xb8,0x21,0x4f,0x64,0x00,
                     0x00,0x00,0xd4,0xfe,0x17,0xff,0xf4,0x01,0x00,0x00,0x00,
                     0x00,0x00,0x00,0x57,0x01,0x00,0x00,0x00,0x00,0x2c,0x60,
                     0xb8,0x0b,0x00,0x00,0xb2,0xb8,0x21,0x4f,0x01,0x07,0x00,
                     0x00,0x81,0x02
                    };
    int rxLength = sizeof(rxBuf);
    HaloUdpStats stats = HALO_UDP_STATS_INIT();
    int stop = 0;
    int timeout = 10; //10 second timeout
    int passed = 0;

    reset_test_state();
    enqueueMsg(&rxBuf[0], rxLength);

    do
    {
        if (myHalo_udp_state_changed())
        {
            stats = get_halo_msg_stats();

            if ((stats.rxBadCrcPkts == 1)&&(stats.rxGoodPkts == 0)&&(!wasRcvFuncCalled()))
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

    printf("Rx Bad Crc Pkt Test: %s\n", passed ? "PASSED" : "FAILED");

    return passed;
}

int txDropTest(void *args)
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
    int timeout = 40; //40 second timeout
    int passed = 0;
    GenericIP socketAddress = GENERIC_IP_INIT();

    reset_test_state();

    //Send a message
    halo_msg_sendto((HaloMessage *) msg, socketAddress);

    do
    {
        if (myHalo_udp_state_changed())
        {
            stats = get_halo_msg_stats();

            if ((stats.txPkts == MAX_REXMIT)&&(stats.txDataPkts == MAX_REXMIT)
                    &&(stats.txConfirmedPkts == 0)&&(stats.txConfirmedBytes == 0)
                    &&(stats.txDroppedPkts == 1)&&(stats.txDataBytes > 0)
                    &&(stats.txBytes > 0)&&(stats.rxAcks == 0)
                    &&(wasDroppedFuncCalled())&&(!wasSentFuncCalled()))
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

    printf("Tx Drop Test: %s\n", passed ? "PASSED" : "FAILED");

    return passed;
}

//txGoodTest Callbacks

static int eachSeqNumDiff = 1;
int txGoodTest_SendCallback(void *args)
{
    static int prevSeqNum = -1; //must be negative before starting?
    int passed = 0;
    HaloUdpEventData *sendEventData = (HaloUdpEventData *) args;
    MyHaloUdpHeader *header = (MyHaloUdpHeader *) sendEventData->data;

    if(prevSeqNum != header->seqNum)
    {
        //Confirmed that sequence changes
        prevSeqNum = header->seqNum;
        passed = 1;
    }

    if(!passed)
    {
        eachSeqNumDiff = 0;
    }

    return passed;
}

int txGoodTest(void *args)
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
    int timeout = 40; //40 second timeout
    int passed = 0;
    GenericIP socketAddress = GENERIC_IP_INIT();

    eachSeqNumDiff = 1; //Makes sure this state variable starts out true

    reset_test_state();
    setTestSendCallback(txGoodTest_SendCallback);

    enableAckGenToRxData();
    enableSessionResponseData();

    //Send a message
    halo_msg_sendto((HaloMessage *) msg, socketAddress);
    halo_msg_sendto((HaloMessage *) msg, socketAddress);
    halo_msg_sendto((HaloMessage *) msg, socketAddress);
    halo_msg_sendto((HaloMessage *) msg, socketAddress);

    do
    {
        if (myHalo_udp_state_changed())
        {
            stats = get_halo_msg_stats();

            /*if(wasSendFuncCalled())
            {
                int sendPassed = getLastTestSendCallbackResult();
                setSendFundCalled(0);

                if(!sendPassed)
                {
                    stop = 1; //quit if any of the finalized tx packets fail evaluation
                }

            }*/

            //quit if any of the finalized tx packets fail evaluation
            if(!eachSeqNumDiff)
            {
                stop = 1;
            }

            if ((stats.txPkts == 4)&&(stats.txDataPkts == 4)
                    &&(stats.txConfirmedPkts == 4)&&(stats.txConfirmedBytes > 0)
                    &&(stats.txDroppedPkts == 0)&&(stats.txDataBytes > 0)
                    &&(stats.txBytes > 0)&&(stats.rxAcks == 4)
                    &&(!wasDroppedFuncCalled())&&(wasSentFuncCalled())
                    &&(getSendFuncCalled() == 4)&&(eachSeqNumDiff))
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

    printf("Tx Good Test: %s\n", passed ? "PASSED" : "FAILED");

    return passed;
}
