#include <time.h>
#include <string.h>
#include <stdio.h>

#include "HaloMessageTypes.h"
#include "HaloMsgHelperFunctions.h"
#include "DynamicVitals.h"
#include "PanicMsg.h"
#include "UdpLib/halo_udp_comm.h"

#include "client_test_impl.h"

void sendDynamicVitalsPkt(void)
{
    DynamicVitals dynamicVitalsMsg = {0};
    uint16 steps[] = {1,2,3,4,5,6,7,8,9,10};
    uint16 activities[] = {1,2,3,4,5,6,7,8,9,10};
    uint8 dynamicVitalsBuffer[sizeof(DynamicVitals)];

    //Set the base parameters for the message
    init_base_message(ALL_DATA_DYNAMIC, 3, time(NULL), 518, (HaloMessage *) &dynamicVitalsMsg);

    dynamicVitalsMsg.stepData.sampleFrequency = 60;
    dynamicVitalsMsg.stepData.updateFrequency = 10;

    dynamicVitalsMsg.stepData.numberOfEntries = (sizeof(steps) / sizeof(uint16));
    memcpy(dynamicVitalsMsg.stepData.steps, steps, sizeof(steps));

    dynamicVitalsMsg.activityData.sampleFrequency = 60;
    dynamicVitalsMsg.activityData.updateFrequency = 10;

    dynamicVitalsMsg.activityData.numberOfEntries = (sizeof(activities) / sizeof(uint16));
    memcpy(dynamicVitalsMsg.activityData.activities, activities, sizeof(activities));

    dynamicVitalsMsg.battInfo.timeRemaining = 50;
    dynamicVitalsMsg.battInfo.battPercentage = 17;
    dynamicVitalsMsg.battInfo.charging = 1;
    dynamicVitalsMsg.battInfo.plugged = 1;

    dynamicVitalsMsg.currentLocation.latitude = 860;
    dynamicVitalsMsg.currentLocation.longitude = -321;
    dynamicVitalsMsg.currentLocation.altitude = 315;
    dynamicVitalsMsg.currentLocation.accuracy = 109;

    //Compress contents of structure into a minimal byte stream for transmission
    pack_DynamicVitals(&dynamicVitalsMsg, dynamicVitalsBuffer);

    //Do a generic send with vitals msg
    halo_msg_send((HaloMessage *) dynamicVitalsBuffer);
}

void sendPanicMsgPkt(void)
{
    PanicMsg panicMsg = {0};
    //Set the base parameters for the message
    init_base_message(CRITICAL_ALERT, 7, time(NULL), 317, (HaloMessage *) &panicMsg);
    panicMsg.criticalAlertBaseMsg.criticalAlertType = PANIC;

    //Location
    panicMsg.criticalAlertBaseMsg.currentLocation.latitude = -750;
    panicMsg.criticalAlertBaseMsg.currentLocation.longitude = 132;
    panicMsg.criticalAlertBaseMsg.currentLocation.altitude = -493;
    panicMsg.criticalAlertBaseMsg.currentLocation.accuracy = 999;

    //Do a generic send with panic msg
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

    printf("\n");
}
