#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include "General/types.h"

#include "UdpLib/myHaloUdp.h"
#include "UdpLib/halo_udp_comm.h"
#include "UdpLib/halo_udp_stats.h"
#include "UdpLib/crc16.h"

#include "halo_udp_tests.h"
#include "test_msg_queue.h"
#include "test_control_interface.h"

#define TRUE  1
#define FALSE 0

static uint8 serverDebug = 0;
static uint8 runThreads = 1;
static HaloUdpUserData haloUdpUserData;

//Tells if these callbacks were invoked
static int udp_msg_rcvd_called    = 0;
static int udp_msg_sent_called    = 0;
static int udp_msg_dropped_called = 0;
static int generateAcks = 0;
static int generateSessionRestarted = 0;
static int txSeqNum = 0;

//Using local function pointers for additional test mgmt info
static TestFunction test_rcvd_callback = NULL;
static int last_test_rcvd_callback_result = 0;
void setTestRcvdCallback(TestFunction testFunction)
{
    test_rcvd_callback = testFunction;
}
int getLastTestRcvdCallbackResult()
{
    return last_test_rcvd_callback_result;
}

static TestFunction test_sent_callback = NULL;
static int last_test_sent_callback_result = 0;
void setTestSentCallback(TestFunction testFunction)
{
    test_sent_callback = testFunction;
}
int getLastTestSentCallbackResult()
{
    return last_test_sent_callback_result;
}

static TestFunction test_dropped_callback = NULL;
static int last_test_dropped_callback_result = 0;
void setTestDroppedCallback(TestFunction testFunction)
{
    test_dropped_callback = testFunction;
}
int getLastTestDroppedCallbackResult()
{
    return last_test_dropped_callback_result;
}

static int udp_msg_send_called    = 0;
static TestFunction test_send_callback = NULL;
static int last_test_send_callback_result = 0;
void setTestSendCallback(TestFunction testFunction)
{
    test_send_callback = testFunction;
}
int getLastTestSendCallbackResult()
{
    return last_test_send_callback_result;
}
void setSendFuncCalled(int called)
{
    udp_msg_send_called = called;
}

int getSendFuncCalled()
{
    return udp_msg_send_called;
}

int wasSendFuncCalled()
{
    return udp_msg_send_called;
}

//Helper functions for myHalo UDP status Indicators
void setRcvFuncCalled(int called)
{
    udp_msg_rcvd_called = called;
}

int getRcvFuncCalled()
{
    return udp_msg_rcvd_called;
}

int wasRcvFuncCalled()
{
    return udp_msg_rcvd_called;
}

void setSentFuncCalled(int called)
{
    udp_msg_sent_called = called;
}

int getSentFuncCalled()
{
    return udp_msg_sent_called;
}

int wasSentFuncCalled()
{
    return udp_msg_sent_called;
}

void setDroppedFuncCalled(int called)
{
    udp_msg_dropped_called = called;
}

int wasDroppedFuncCalled()
{
    return udp_msg_dropped_called;
}

void enableAckGenToRxData()
{
    generateAcks = 1;
}

void disableAckGenToRxData()
{
    generateAcks = 0;
}

void enableSessionResponseData()
{
    generateSessionRestarted = 1;
}

void disableSessionResponseData()
{
    generateSessionRestarted = 0;
}

int getNextTxSeqNum()
{
    return txSeqNum++;
}

HaloUdpAckPkt acknowledgeMessage(MyHaloUdpHeader *header, int generateSessionRestarted)
{
    //Send notification of receipt of ack to server
    HaloUdpAckPkt ackPkt = HALO_UDP_ACK_PKT_INIT();

    ackPkt.header.status    |= MSG_RECEIVED_ACK;

    if(generateSessionRestarted)
    {
        //Handles notification that a new remote session has started
        if ((header->status & NEW_SESSION) == NEW_SESSION)
        {
            //Indicate confirmation preparing for the new remote session
            ackPkt.header.status |= SESSION_RESTARTED;
        }
    }

    ackPkt.header.ackSeqNum  = header->seqNum;
    ackPkt.header.seqNum     = getNextTxSeqNum();
    ackPkt.crc = hdlcFcs16(hdlc_init_fcs16, (const uint8 *) &ackPkt, sizeof(ackPkt) - sizeof(uint16));

    return ackPkt;
}

//Return any data we want to inject into the myHalo UDP stack here
int test_recv(uint8 *data, int max_len)
{
    uint8 *dataPtr = NULL;
    int dataLen = 0;

    if (hasMsgs())
    {
        peekMsg((void **) &dataPtr, &dataLen);
        memcpy(data, dataPtr, (dataLen < max_len) ? dataLen : max_len);
        dequeueMsg();
    }

    return dataLen;
}

//Process any data the myHalo UDP stack has generated.
int test_send(uint8 *data, int len)
{
    uint8 txBuf[2048];
    int txLength = 0;

    //Include in the ack
    if(generateAcks)
    {
        HaloUdpAckPkt ackPkt;

        ackPkt = acknowledgeMessage((MyHaloUdpHeader *) data, generateSessionRestarted);

        //NOTE: This will only work one packet at a time
        memcpy(txBuf,&ackPkt, sizeof(ackPkt));
        txLength = sizeof(ackPkt);
        enqueueMsg(txBuf, txLength);
    }

    if(test_send_callback)
    {
        HaloUdpEventData sendEventData = HALO_UDP_EVENT_DATA_INIT();

        sendEventData.data = data;
        sendEventData.dataLength = len;

        last_test_send_callback_result = test_send_callback(&sendEventData);
    }

    udp_msg_send_called++;

    return len;
}

//Data that is to be transmitted is what is passed in
void test_myhalo_udp_msg_rcvd(void *data)
{
    HaloUdpEventData *rcvdEventData = (HaloUdpEventData *) data;

    if(test_rcvd_callback)
    {
        last_test_rcvd_callback_result = test_rcvd_callback(rcvdEventData);
    }

    udp_msg_rcvd_called++;
}

void test_myhalo_udp_msg_sent(void *data)
{
    HaloUdpEventData *sentEventData = (HaloUdpEventData *) data;

    if(test_sent_callback)
    {
        last_test_sent_callback_result = test_sent_callback(sentEventData);
    }

    udp_msg_sent_called++;
}

void test_myhalo_udp_msg_dropped(void *data)
{
    HaloUdpEventData *droppedEventData = (HaloUdpEventData *) data;

    if(test_dropped_callback)
    {
        last_test_dropped_callback_result = test_dropped_callback(droppedEventData);
    }

    udp_msg_dropped_called++;
}

int doTest()
{
    return runAllTestItems(&myHaloUdpProtocolTesting);
}

void * testThreadHandler(void *arg)
{
    int passed = -1;

    passed = doTest();

    *(int *) arg = passed;

    runThreads = 0;

    return arg;
}

int run_test(unsigned char debug, char *port)
{
    uint8 actAsServer = 1;
    pthread_t testThr;
    int testResult = -1;

    serverDebug = debug;

    //Sets up Notifiers
    haloUdpUserData.actAsServer     = actAsServer;
    haloUdpUserData.debug           = debug;
    haloUdpUserData.hostname        = NULL;
    haloUdpUserData.port            = port;
    haloUdpUserData.msg_rx_received = test_myhalo_udp_msg_rcvd;
    haloUdpUserData.msg_tx_sent     = test_myhalo_udp_msg_sent;
    haloUdpUserData.msg_tx_dropped  = test_myhalo_udp_msg_dropped;

    halo_msg_init(&haloUdpUserData);

    if (pthread_create(&testThr, NULL, &testThreadHandler, &testResult))
    {
        printf("Could not create thread\n");
        exit(-1);
    }

    while (runThreads)
    {
        halo_msg_tick();
        usleep(2000); // 2ms sleep
    }

    runThreads = 0;

    if (pthread_join(testThr, NULL))
    {
        printf("Could not join thread\n");
        exit(-1);
    }

    return testResult;
}

