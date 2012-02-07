#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>

#include "types.h"
#include "mpers/Location.h"
#include "mpers/DynamicVitalsMsg.h"
#include "mpers/CriticalAlertMsg.h"
#include "mpers/HaloMsgHelperFunctions.h"
#include "mpers/HaloMessageTypes.h"
#include "UdpLib/halo_udp_comm.h"
#include "UdpLib/myHaloUdp.h"
#include "client_tests.h"

static uint8 runThreads  = 1;
static uint8 clientDebug = 0;
static HaloUdpUserData haloUdpUserData;

void client_myhalo_udp_msg_rcvd(void *data)
{
    //HaloMessage *msg = (HaloMessage *) data;
    HaloUdpEventData *rcvEventData = (HaloUdpEventData *) data;
    HaloMessage *msg = (HaloMessage *) rcvEventData->data;

    if (msg->commandType == ALL_DATA_DYNAMIC)
    {
        DynamicVitalsMsg dynamicVitalsMsg;

        //Read the structure correctly
        unpack_DynamicVitalsMsg(msg, &dynamicVitalsMsg);

        if (clientDebug)
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
        if (clientDebug)
        {
            CriticalAlertMsg *criticalAlertMsg = (CriticalAlertMsg *) msg;
            printf("Critical Alert msg Received!\n");
            printf("Critical Alert type: %d\n", criticalAlertMsg->criticalAlertType);
            printf("Location Size: %lu\n", sizeof(Location));
            printf("HaloMessage Size: %lu\n", sizeof(HaloMessage));
            printf("CriticalAlert Size: %lu\n", sizeof(CriticalAlertMsg));
        }
    }
}

void client_myhalo_udp_msg_sent(void *data)
{
    //HaloUdpEventData *rcvEventData = (HaloUdpEventData *) data;
    //HaloMessage *msg = (HaloMessage *) rcvEventData->data;
    HaloMessage *msg = (HaloMessage *) data;

    if (msg->commandType == ALL_DATA_DYNAMIC)
    {
        if (clientDebug)
        {
            printf("Dynamic Vitals msg sent successfully!\n");
        }
    }
    else if (msg->commandType == CRITICAL_ALERT)
    {
        if (clientDebug)
            printf("Critical Alert msg sent successfully!\n");
    }
}

void client_myhalo_udp_msg_dropped(void *data)
{
    HaloMessage *msg = (HaloMessage *) data;

    if (msg->commandType == ALL_DATA_DYNAMIC)
    {
        ////Set this up to never give up
        //halo_msg_send(data);

        if (clientDebug)
            printf("Dynamic Vitals msg dropped!\n");
    }
}

void * testThreadHandler(void *arg)
{
    TestInfo *pTestInfo = (TestInfo *) arg;

    assert(pTestInfo);

    pTestInfo->testResult = runTestItem(&packetTesting, pTestInfo->testNumber, pTestInfo->testArgs);
    runThreads = 0;

    return NULL;
}

int run_client(unsigned char debug, char *hostname, char *port, int testNumber, char *testArgs, HaloUdpCommDbg dbgParams)
{
    uint8 actAsServer = 0;
    pthread_t testThr;
    TestInfo testInfo = TEST_INFO_INIT(testNumber-1, testArgs);

    clientDebug = debug;

    //Sets up Notifiers
    haloUdpUserData.actAsServer     = actAsServer;
    haloUdpUserData.debug           = debug;
    haloUdpUserData.hostname        = hostname;
    haloUdpUserData.port            = port;
    haloUdpUserData.msg_rx_received = client_myhalo_udp_msg_rcvd;
    haloUdpUserData.msg_tx_sent     = client_myhalo_udp_msg_sent;
    haloUdpUserData.msg_tx_dropped  = client_myhalo_udp_msg_dropped;

    halo_msg_init(&haloUdpUserData);
    set_halo_udp_comm_dbg(dbgParams);

    if (pthread_create(&testThr, NULL, &testThreadHandler, &testInfo))
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
