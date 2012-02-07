#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>

#include "types.h"

#include "mpers/Location.h"
#include "mpers/DynamicVitalsMsg.h"
#include "mpers/CriticalAlertMsg.h"
#include "UdpLib/myHaloUdp.h"
#include "mpers/HaloMsgHelperFunctions.h"
#include "mpers/HaloMessageTypes.h"
#include "server_tests.h"

#include "UdpLib/halo_udp_comm.h"

static uint8 serverDebug = 0;
static uint8 runThreads = 1;
static HaloUdpUserData haloUdpUserData;

//Data that is to be transmitted is what is passed in
void server_myhalo_udp_msg_rcvd(void *data)
{
    //HaloMessage *msg = (HaloMessage *) data;
    HaloUdpEventData *rcvEventData = (HaloUdpEventData *) data;
    HaloMessage *msg = (HaloMessage *) rcvEventData->data;

    if (msg->commandType == ALL_DATA_DYNAMIC)
    {
        DynamicVitalsMsg dynamicVitalsMsg;

        //Send back the same data
        //halo_msg_send(msg);
        halo_msg_sendto(msg, rcvEventData->socketAddress);

        //Read the structure correctly
        unpack_DynamicVitalsMsg(msg, &dynamicVitalsMsg);

        if (serverDebug)
        {
            printf("Dynamic Vitals msg received!\n");
            printf("Step sample Rate: %d\n", dynamicVitalsMsg.stepData.sampleRate);
            printf("Activity sample Rate: %d\n", dynamicVitalsMsg.activityData.sampleRate);
            printf("SrcIp: 0x%x\n", rcvEventData->socketAddress.address);
            printf("SrcPort: %d\n", rcvEventData->socketAddress.port);
        }
    }
    else if (msg->commandType == CRITICAL_ALERT)
    {
        if (serverDebug)
        {
            CriticalAlertMsg *criticalAlertMsg = (CriticalAlertMsg *) msg;
            printf("Critical Alert msg Received!\n");
            printf("Critical Alert type: %d\n", criticalAlertMsg->criticalAlertType);
        }
    }
}

void server_myhalo_udp_msg_sent(void *data)
{
    HaloMessage *msg = (HaloMessage *) data;

    if (msg->commandType == ALL_DATA_DYNAMIC)
    {
        if (serverDebug)
            printf("Dynamic Vitals msg sent successfully!\n");
    }
    else if (msg->commandType == CRITICAL_ALERT)
    {
        if (serverDebug)
            printf("Critical Alert msg sent successfully!\n");
    }
}

void server_myhalo_udp_msg_dropped(void *data)
{
    HaloMessage *msg = (HaloMessage *) data;

    if (msg->commandType == ALL_DATA_DYNAMIC)
    {
        //halo_msg_send(data);

        if (serverDebug)
            printf("Dynamic Vitals msg dropped!\n");
    }
}

void * serverTestThreadHandler(void *arg)
{
    TestInfo *pTestInfo = (TestInfo *) arg;

    assert(pTestInfo);

    pTestInfo->testResult = runTestItem(&serverPacketTesting, pTestInfo->testNumber, pTestInfo->testArgs);
    runThreads = 0;

    return NULL;
}

int run_server(unsigned char debug, char *port, int testNumber, char *testArgs, HaloUdpCommDbg dbgParams)
{
    uint8 actAsServer = 1;
    pthread_t testThr;
    TestInfo testInfo = TEST_INFO_INIT(testNumber-1, testArgs);

    serverDebug = debug;

    //Sets up Notifiers
    haloUdpUserData.actAsServer     = actAsServer;
    haloUdpUserData.debug           = debug;
    haloUdpUserData.hostname        = NULL;
    haloUdpUserData.port            = port;
    haloUdpUserData.msg_rx_received = server_myhalo_udp_msg_rcvd;
    haloUdpUserData.msg_tx_sent     = server_myhalo_udp_msg_sent;
    haloUdpUserData.msg_tx_dropped  = server_myhalo_udp_msg_dropped;

    halo_msg_init(&haloUdpUserData);
    set_halo_udp_comm_dbg(dbgParams);

    if (pthread_create(&testThr, NULL, &serverTestThreadHandler, &testInfo))
    {
        printf("Could not create thread\n");
        exit(-1);
    }

    while (runThreads)
    {
        halo_msg_tick();
        usleep(2000);
    }

    runThreads = 0;

    if (pthread_join(testThr, NULL))
    {
        printf("Could not join thread\n");
        exit(-1);
    }

    return testInfo.testResult;
}

