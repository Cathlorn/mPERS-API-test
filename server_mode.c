#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

#include "types.h"

#include "Location.h"
#include "DynamicVitals.h"
#include "CriticalAlert.h"
#include "UdpLib/myHaloUdp.h"
#include "HaloMsgHelperFunctions.h"
#include "HaloMessageTypes.h"
#include "server_tests.h"

#include "UdpLib/halo_udp_comm.h"

static uint8 serverDebug = 0;
static uint8 runThreads = 1;
static HaloUdpUserData haloUdpUserData;

//Data that is to be transmitted is what is passed in
void server_myhalo_udp_msg_rcvd(void *data)
{
    //HaloMessage *msg = (HaloMessage *) data;
    HaloUdpRcvEventData *rcvEventData = (HaloUdpRcvEventData *) data;
    HaloMessage *msg = (HaloMessage *) rcvEventData->data;

    if (msg->commandType == ALL_DATA_DYNAMIC)
    {
        DynamicVitals dynamicVitals;

        //Send back the same data
        //halo_msg_send(msg);
        halo_msg_sendto(msg, &rcvEventData->socketAddress, rcvEventData->socketAddressLength);

        //Read the structure correctly
        unpack_DynamicVitals(msg, &dynamicVitals);

        if (serverDebug)
        {
            printf("Dynamic Vitals msg received!\n");
            printf("Step upload Frequency: %d\n", dynamicVitals.stepData.updateFrequency);
            printf("Activity sample Frequency: %d\n", dynamicVitals.activityData.sampleFrequency);
            printf("SrcIp: 0x%x\n", rcvEventData->socketAddress.sin_addr.s_addr);
            printf("SrcPort: %d\n", rcvEventData->socketAddress.sin_port);
        }
    }
    else if (msg->commandType == CRITICAL_ALERT)
    {
        if (serverDebug)
        {
            CriticalAlert *criticalAlertMsg = (CriticalAlert *) msg;
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
    while (runThreads)
    {
        printTestItemMenu(&serverPacketTesting);
        getTestItemChoice(&serverPacketTesting);
    }

    return NULL;
}

void run_server(unsigned char debug, char *port)
{
    uint8 actAsServer = 1;
    pthread_t testThr;

    serverDebug = debug;
    if (pthread_create(&testThr, NULL, &serverTestThreadHandler, NULL))
    {
        printf("Could not create thread\n");
        exit(-1);
    }

    //Sets up Notifiers
    haloUdpUserData.actAsServer     = actAsServer;
    haloUdpUserData.debug           = debug;
    haloUdpUserData.hostname        = NULL;
    haloUdpUserData.port            = port;
    haloUdpUserData.msg_rx_received = server_myhalo_udp_msg_rcvd;
    haloUdpUserData.msg_tx_sent     = server_myhalo_udp_msg_sent;
    haloUdpUserData.msg_tx_dropped  = server_myhalo_udp_msg_dropped;

    halo_msg_init(&haloUdpUserData);

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
}

