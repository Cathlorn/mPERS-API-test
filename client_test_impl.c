#include <time.h>
#include <string.h>
#include <stdio.h>

#include "HaloMessageTypes.h"
#include "HaloMsgHelperFunctions.h"
#include "DynamicVitalsMsg.h"
#include "CriticalAlertMsg.h"
#include "UdpLib/halo_udp_comm.h"

#include "client_test_impl.h"

void sendDynamicVitalsPkt(void)
{
    DynamicVitalsMsg dynamicVitalsMsg = DYNAMIC_VITALS_MSG_INIT();
    uint16 steps[] = {1,2,3,4,5,6,7,8,9,10};
    uint16 activities[] = {1,2,3,4,5,6,7,8,9,10};
    uint8 dynamicVitalsBuffer[sizeof(DynamicVitalsMsg)]; //Created to be this size since the packed version should always be the same or smaller than the full msg version

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

    //Do a generic send with vitals msg
    halo_msg_send((HaloMessage *) dynamicVitalsBuffer);
}

void sendPanicMsgPkt(void)
{
    CriticalAlertMsg panicMsg = CRITICAL_ALERT_MSG_INIT(PANIC);

    init_base_message(panicMsg.baseMessage.commandType, 7, time(NULL), 317, (HaloMessage *) &panicMsg);
    panicMsg.currentLocation.latitude = -750;
    panicMsg.currentLocation.longitude = 132;
    panicMsg.currentLocation.elevation = -493;
    panicMsg.currentLocation.accuracy = 999;

    //Do a generic send with panic msg
    halo_msg_send((HaloMessage *) &panicMsg);
}

void sendFallMsgPkt(void)
{
    CriticalAlertMsg fallMsg = CRITICAL_ALERT_MSG_INIT(FALL);

    init_base_message(fallMsg.baseMessage.commandType, 7, time(NULL), 317, (HaloMessage *) &fallMsg);
    fallMsg.currentLocation.latitude = -800;
    fallMsg.currentLocation.longitude = 460;
    fallMsg.currentLocation.elevation = -770;
    fallMsg.currentLocation.accuracy = 23;

    //Do a generic send with panic msg
    halo_msg_send((HaloMessage *) &fallMsg);
}

void sendBurst(void)
{
    CriticalAlertMsg panicMsg = CRITICAL_ALERT_MSG_INIT(PANIC);

    init_base_message(panicMsg.baseMessage.commandType, 7, time(NULL), 317, (HaloMessage *) &panicMsg);
    panicMsg.currentLocation.latitude = 590;
    panicMsg.currentLocation.longitude = 823;
    panicMsg.currentLocation.elevation = -215;
    panicMsg.currentLocation.accuracy = 217;

    //Do a generic send with panic msg
    halo_msg_send((HaloMessage *) &panicMsg);
    halo_msg_send((HaloMessage *) &panicMsg);
    halo_msg_send((HaloMessage *) &panicMsg);
}

void start_new_halo_udp_session(void)
{
    halo_msg_new_session(0);
    printf("Halo UDP New Session Created!\n");
}

void show_halo_udp_sessions(void)
{
    int sessionCount;
    int i;

    sessionCount = halo_msg_session_count();

    if (sessionCount <= 0)
    {
        printf("No session data available.\n");
    }
    else
    {
        for (i=0; i < sessionCount; i++)
        {
            halo_msg_report_session(i);
        }
    }
}

void halo_udp_loopback_enable(void)
{
    HaloUdpCommDbg dbgTestCtrls = get_halo_udp_comm_dbg();

    dbgTestCtrls.loopback = 1;

    set_halo_udp_comm_dbg(dbgTestCtrls);

    printf("Halo UDP Loopback Enabled\n");
}

void halo_udp_loopback_disable(void)
{
    HaloUdpCommDbg dbgTestCtrls = get_halo_udp_comm_dbg();

    dbgTestCtrls.loopback = 0;

    set_halo_udp_comm_dbg(dbgTestCtrls);

    printf("Halo UDP Loopback Enabled\n");
}

void halo_udp_bad_crc_enable(void)
{
    HaloUdpCommDbg dbgTestCtrls = get_halo_udp_comm_dbg();

    dbgTestCtrls.badCrc = 1;

    set_halo_udp_comm_dbg(dbgTestCtrls);

    printf("Halo UDP BadCrc Enabled\n");
}

void halo_udp_bad_crc_disable(void)
{
    HaloUdpCommDbg dbgTestCtrls = get_halo_udp_comm_dbg();

    dbgTestCtrls.badCrc = 0;

    set_halo_udp_comm_dbg(dbgTestCtrls);

    printf("Halo UDP BadCrc Enabled\n");
}

void halo_udp_never_ack_enable(void)
{
    HaloUdpCommDbg dbgTestCtrls = get_halo_udp_comm_dbg();

    dbgTestCtrls.neverAck = 1;

    set_halo_udp_comm_dbg(dbgTestCtrls);

    printf("Halo UDP NeverAck Enabled\n");
}

void halo_udp_never_ack_disable(void)
{
    HaloUdpCommDbg dbgTestCtrls = get_halo_udp_comm_dbg();

    dbgTestCtrls.neverAck = 0;

    set_halo_udp_comm_dbg(dbgTestCtrls);

    printf("Halo UDP NeverAck Enabled\n");
}

void halo_udp_never_tx_drop_enable(void)
{
    HaloUdpCommDbg dbgTestCtrls = get_halo_udp_comm_dbg();

    dbgTestCtrls.neverTxDrop = 1;

    set_halo_udp_comm_dbg(dbgTestCtrls);

    printf("Halo UDP NeverTxDrop Enabled\n");
}

void halo_udp_never_tx_drop_disable(void)
{
    HaloUdpCommDbg dbgTestCtrls = get_halo_udp_comm_dbg();

    dbgTestCtrls.neverTxDrop = 0;

    set_halo_udp_comm_dbg(dbgTestCtrls);

    printf("Halo UDP NeverTxDrop Enabled\n");
}

void halo_udp_duplicate_tx_enable(void)
{
    HaloUdpCommDbg dbgTestCtrls = get_halo_udp_comm_dbg();

    dbgTestCtrls.duplicateTx = 1;

    set_halo_udp_comm_dbg(dbgTestCtrls);

    printf("Halo UDP DuplicateTx Enabled\n");
}

void halo_udp_duplicate_tx_disable(void)
{
    HaloUdpCommDbg dbgTestCtrls = get_halo_udp_comm_dbg();

    dbgTestCtrls.duplicateTx = 0;

    set_halo_udp_comm_dbg(dbgTestCtrls);

    printf("Halo UDP DuplicateTx Enabled\n");
}

void halo_udp_out_of_seq_tx_enable(void)
{
    HaloUdpCommDbg dbgTestCtrls = get_halo_udp_comm_dbg();

    dbgTestCtrls.outOfSeqTx = 1;

    set_halo_udp_comm_dbg(dbgTestCtrls);

    printf("Halo UDP OutOfSeqTx Enabled\n");
}

void halo_udp_out_of_seq_tx_disable(void)
{
    HaloUdpCommDbg dbgTestCtrls = get_halo_udp_comm_dbg();

    dbgTestCtrls.outOfSeqTx = 0;

    set_halo_udp_comm_dbg(dbgTestCtrls);

    printf("Halo UDP OutOfSeqTx Enabled\n");
}

void halo_udp_spotty_comm_control(void)
{
    HaloUdpCommDbg dbgTestCtrls = get_halo_udp_comm_dbg();
    int percentFail = 80;

    printf("Enter the percentage of failure you want. (0 to disable): ");
    scanf("%d", &percentFail);
    printf("%d %% Failure selected.\n", percentFail);

    if(percentFail)
    {
        printf("Halo UDP SpottyRx Enabled\n");
        //dbgTestCtrls.spottyRx = 1;
        dbgTestCtrls.spottyRx = percentFail;
    }
    else
    {
        printf("Halo UDP SpottyRx Disabled\n");
        //dbgTestCtrls.spottyRx = 0;
        dbgTestCtrls.spottyRx = percentFail;
    }
    set_halo_udp_comm_dbg(dbgTestCtrls);
}

void halo_udp_print_dbg_comm_status(void)
{
    HaloUdpCommDbg dbgTestCtrls = get_halo_udp_comm_dbg();

    printf("\n");

    if (dbgTestCtrls.loopback)
        printf("Loopback   : Enabled\n");
    else
        printf("Loopback   : Disabled\n");

    if (dbgTestCtrls.badCrc)
        printf("BadCrc     : Enabled\n");
    else
        printf("BadCrc     : Disabled\n");

    if (dbgTestCtrls.neverAck)
        printf("NeverAck   : Enabled\n");
    else
        printf("NeverAck   : Disabled\n");

    if (dbgTestCtrls.neverTxDrop)
        printf("NeverTxDrop: Enabled\n");
    else
        printf("NeverTxDrop: Disabled\n");

    if (dbgTestCtrls.duplicateTx)
        printf("DuplicateTx: Enabled\n");
    else
        printf("DuplicateTx: Disabled\n");

    if (dbgTestCtrls.outOfSeqTx)
        printf("OutOfSeqTx : Enabled\n");
    else
        printf("OutOfSeqTx : Disabled\n");

    /*if (dbgTestCtrls.spottyRx)
        printf("SpottyRx   : Enabled\n");
    else
        printf("SpottyRx   : Disabled\n"); */

    if (dbgTestCtrls.spottyRx)
        printf("SpottyRx   : %d %% Failure (Enabled)\n", dbgTestCtrls.spottyRx);
    else
        printf("SpottyRx   : %d %% Failure (Disabled)\n", dbgTestCtrls.spottyRx);

    printf("\n");
}
