#include <time.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include "mpers/HaloMessageTypes.h"
#include "mpers/HaloMsgHelperFunctions.h"
#include "mpers/DynamicVitalsMsg.h"
#include "mpers/CriticalAlertMsg.h"
#include "UdpLib/halo_udp_comm.h"
#include "UdpLib/halo_udp_stats.h"

#include "client_test_impl.h"

int sendDynamicVitalsPkt(void *args)
{
    int passed = 0;
    DynamicVitalsMsg dynamicVitalsMsg = DYNAMIC_VITALS_MSG_INIT();
    uint16 steps[] = {1,2,3,4,5,6,7,8,9,10};
    uint16 activities[] = {1,2,3,4,5,6,7,8,9,10};
    uint8 dynamicVitalsBuffer[sizeof(DynamicVitalsMsg)]; //Created to be this size since the packed version should always be the same or smaller than the full msg version
    HaloUdpStats stats = HALO_UDP_STATS_INIT();
    int stop = 0;
    int timeout = 10; //second timeout
    int numMsgsToSend = 1;
    int i = 0;

    if(args)
    {
        numMsgsToSend = atoi((char *) args);
        timeout *= numMsgsToSend;
    }

    //Set the base parameters for the message
    init_base_message(ALL_DATA_DYNAMIC, 3, time(NULL), 518, (HaloMessage *) &dynamicVitalsMsg);

    dynamicVitalsMsg.stepData.sampleRate = 60;

    dynamicVitalsMsg.stepData.numberOfEntries = (sizeof(steps) / sizeof(uint16));
    memcpy(dynamicVitalsMsg.stepData.steps, steps, sizeof(steps));

    dynamicVitalsMsg.activityData.sampleRate = 60;

    dynamicVitalsMsg.activityData.numberOfEntries = (sizeof(activities) / sizeof(uint16));
    memcpy(dynamicVitalsMsg.activityData.activities, activities, sizeof(activities));

    dynamicVitalsMsg.battInfo.minRemaining = 50;
    dynamicVitalsMsg.battInfo.battPercentage = 17;
    dynamicVitalsMsg.battInfo.charging = 1;
    dynamicVitalsMsg.battInfo.plugged = 1;
    dynamicVitalsMsg.battInfo.battVoltage = 3000;

    dynamicVitalsMsg.currentLocation.latitude = 860;
    dynamicVitalsMsg.currentLocation.longitude = -321;
    dynamicVitalsMsg.currentLocation.elevation = 315;
    dynamicVitalsMsg.currentLocation.accuracy = 109;

    //Compress contents of structure into a minimal byte stream for transmission
    pack_DynamicVitalsMsg(&dynamicVitalsMsg, dynamicVitalsBuffer);

    //Clear stats
    halo_msg_reset_stats();

    for(i = 0; i < numMsgsToSend; i++)
    {
        //Do a generic send with vitals msg
        halo_msg_send((HaloMessage *) dynamicVitalsBuffer);
    }

    do
    {
        stats = get_halo_msg_stats();

        if ((stats.txPkts == numMsgsToSend)&&(stats.txDataPkts == numMsgsToSend)
                &&(stats.txConfirmedPkts == numMsgsToSend)&&(stats.txConfirmedBytes > 0)
                &&(stats.txDroppedPkts == 0)&&(stats.txDataBytes > 0)
                &&(stats.txBytes > 0)&&(stats.rxAcks == numMsgsToSend))
        {
            //Confirmed there is a stop
            stop = 1;
            passed = 1;
        }

        printf("%d of %d confirmed sent (%d transmitted)\n", stats.txConfirmedPkts, numMsgsToSend, stats.txDataPkts);

        sleep(1);
        timeout--;
    }
    while ((!stop)&&(timeout));

    if (!timeout)
    {
        printf("Test Timed Out!\n");
    }

    printf("Dynamic Vitals Test: %s\n", passed ? "PASSED" : "FAILED");

    return passed;
}

int sendPanicMsgPkt(void *args)
{
    int passed = 0;
    CriticalAlertMsg panicMsg = CRITICAL_ALERT_MSG_INIT(PANIC);

    HaloUdpStats stats = HALO_UDP_STATS_INIT();
    int stop = 0;
    int timeout = 10; //second timeout
    int numMsgsToSend = 1;
    int i = 0;

    if(args)
    {
        numMsgsToSend = atoi((char *) args);
        timeout *= numMsgsToSend;
    }

    init_base_message(panicMsg.baseMessage.commandType, 7, time(NULL), 317, (HaloMessage *) &panicMsg);
    panicMsg.currentLocation.latitude = -750;
    panicMsg.currentLocation.longitude = 132;
    panicMsg.currentLocation.elevation = -493;
    panicMsg.currentLocation.accuracy = 999;

    //Clear stats
    halo_msg_reset_stats();

    for(i = 0; i < numMsgsToSend; i++)
    {
        halo_msg_send((HaloMessage *) &panicMsg);
    }

    do
    {
        stats = get_halo_msg_stats();

        if ((stats.txPkts == numMsgsToSend)&&(stats.txDataPkts == numMsgsToSend)
                &&(stats.txConfirmedPkts == numMsgsToSend)&&(stats.txConfirmedBytes > 0)
                &&(stats.txDroppedPkts == 0)&&(stats.txDataBytes > 0)
                &&(stats.txBytes > 0)&&(stats.rxAcks == numMsgsToSend))
        {
            //Confirmed there is a stop
            stop = 1;
            passed = 1;
        }

        printf("%d of %d confirmed sent (%d transmitted)\n", stats.txConfirmedPkts, numMsgsToSend, stats.txDataPkts);

        sleep(1);
        timeout--;
    }
    while ((!stop)&&(timeout));

    if (!timeout)
    {
        printf("Test Timed Out!\n");
    }

    printf("Panic Test: %s\n", passed ? "PASSED" : "FAILED");


    return passed;
}

int sendFallMsgPkt(void *args)
{
    int passed = 0;
    CriticalAlertMsg fallMsg = CRITICAL_ALERT_MSG_INIT(FALL);
    HaloUdpStats stats = HALO_UDP_STATS_INIT();
    int stop = 0;
    int timeout = 10; //second timeout
    int numMsgsToSend = 1;
    int i = 0;

    if(args)
    {
        numMsgsToSend = atoi((char *) args);
        timeout *= numMsgsToSend;
    }

    init_base_message(fallMsg.baseMessage.commandType, 7, time(NULL), 317, (HaloMessage *) &fallMsg);
    fallMsg.currentLocation.latitude = -800;
    fallMsg.currentLocation.longitude = 460;
    fallMsg.currentLocation.elevation = -770;
    fallMsg.currentLocation.accuracy = 23;

    //Clear stats
    halo_msg_reset_stats();

    for(i = 0; i < numMsgsToSend; i++)
    {
        halo_msg_send((HaloMessage *) &fallMsg);
    }

    do
    {
        stats = get_halo_msg_stats();

        if ((stats.txPkts == numMsgsToSend)&&(stats.txDataPkts == numMsgsToSend)
                &&(stats.txConfirmedPkts == numMsgsToSend)&&(stats.txConfirmedBytes > 0)
                &&(stats.txDroppedPkts == 0)&&(stats.txDataBytes > 0)
                &&(stats.txBytes > 0)&&(stats.rxAcks == numMsgsToSend))
        {
            //Confirmed there is a stop
            stop = 1;
            passed = 1;
        }

        printf("%d of %d confirmed sent (%d transmitted)\n", stats.txConfirmedPkts, numMsgsToSend, stats.txDataPkts);

        sleep(1);
        timeout--;
    }
    while ((!stop)&&(timeout));

    if (!timeout)
    {
        printf("Test Timed Out!\n");
    }

    printf("Fall Test: %s\n", passed ? "PASSED" : "FAILED");

    return passed;
}

int sendOperatorAckMsgPkt(void *args)
{
    int passed = 0;
    CriticalAlertMsg operatorAckMsg = CRITICAL_ALERT_MSG_INIT(OPERATOR_ACK);
    HaloUdpStats stats = HALO_UDP_STATS_INIT();
    int stop = 0;
    int timeout = 10; //second timeout
    int numMsgsToSend = 1;
    int i = 0;

    if(args)
    {
        numMsgsToSend = atoi((char *) args);
        timeout *= numMsgsToSend;
    }

    init_base_message(operatorAckMsg.baseMessage.commandType, 7, time(NULL), 317, (HaloMessage *) &operatorAckMsg);
    operatorAckMsg.currentLocation.latitude = -800;
    operatorAckMsg.currentLocation.longitude = 460;
    operatorAckMsg.currentLocation.elevation = -770;
    operatorAckMsg.currentLocation.accuracy = 23;

    //Clear stats
    halo_msg_reset_stats();

    for(i = 0; i < numMsgsToSend; i++)
    {
        halo_msg_send((HaloMessage *) &operatorAckMsg);
    }

    do
    {
        stats = get_halo_msg_stats();

        if ((stats.txPkts == numMsgsToSend)&&(stats.txDataPkts == numMsgsToSend)
                &&(stats.txConfirmedPkts == numMsgsToSend)&&(stats.txConfirmedBytes > 0)
                &&(stats.txDroppedPkts == 0)&&(stats.txDataBytes > 0)
                &&(stats.txBytes > 0)&&(stats.rxAcks == numMsgsToSend))
        {
            //Confirmed there is a stop
            stop = 1;
            passed = 1;
        }

        printf("%d of %d confirmed sent (%d transmitted)\n", stats.txConfirmedPkts, numMsgsToSend, stats.txDataPkts);

        sleep(1);
        timeout--;
    }
    while ((!stop)&&(timeout));

    if (!timeout)
    {
        printf("Test Timed Out!\n");
    }

    printf("Operator Acknowledgment Test: %s\n", passed ? "PASSED" : "FAILED");

    return passed;
}

int sendInvalidMsgPkt(void *args)
{
    int passed = 0;
    uint8 invalidMsg[] = {0xaa,0x11,0x33,0x34,0x43,0xbb,0x22,0x00,0x77,0x88,0x99,0x33};
    HaloUdpStats stats = HALO_UDP_STATS_INIT();
    int stop = 0;
    int timeout = 10; //second timeout
    int numMsgsToSend = 1;
    int i = 0;

    if(args)
    {
        numMsgsToSend = atoi((char *) args);
        timeout *= numMsgsToSend;
    }

    //Clear stats
    halo_msg_reset_stats();

    for(i = 0; i < numMsgsToSend; i++)
    {
        halo_msg_send((HaloMessage *) &invalidMsg[0]);
    }

    do
    {
        stats = get_halo_msg_stats();

        if ((stats.txPkts == numMsgsToSend)&&(stats.txDataPkts == numMsgsToSend)
                &&(stats.txConfirmedPkts == numMsgsToSend)&&(stats.txConfirmedBytes > 0)
                &&(stats.txDroppedPkts == 0)&&(stats.txDataBytes > 0)
                &&(stats.txBytes > 0)&&(stats.rxAcks == numMsgsToSend))
        {
            //Confirmed there is a stop
            stop = 1;
            passed = 1;
        }

        printf("%d of %d confirmed sent (%d transmitted)\n", stats.txConfirmedPkts, numMsgsToSend, stats.txDataPkts);

        sleep(1);
        timeout--;
    }
    while ((!stop)&&(timeout));

    if (!timeout)
    {
        printf("Test Timed Out!\n");
    }

    printf("Invalid mPERS Format Msg Test: %s\n", passed ? "PASSED" : "FAILED");

    return passed;
}
