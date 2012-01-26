#include "halo_udp_stats.h"

void resetHaloUdpStats(HaloUdpStats *stats)
{
    //HaloUdpStats clearStats = HALO_UDP_STATS_INIT();
    //*stats = clearStats;

    HALO_UDP_STATS_CLR(stats);
}

uint32 getTxBytes(HaloUdpStats *stats)
{
    return stats->txBytes;
}

uint32 updateTxBytes(HaloUdpStats *stats, uint32 bytes)
{
    stats->txBytes += bytes;

    return stats->txBytes;
}

uint32 getTxConfirmedBytes(HaloUdpStats *stats)
{
    return stats->txConfirmedBytes;
}

uint32 updateTxConfirmedBytes(HaloUdpStats *stats, uint32 bytes)
{
    stats->txConfirmedBytes += bytes;

    return stats->txConfirmedBytes;
}

uint32 getTxPkts(HaloUdpStats *stats)
{
    return stats->txPkts;
}

uint32 updateTxPkts(HaloUdpStats *stats, uint32 pkts)
{
    stats->txPkts += pkts;

    return stats->txPkts;
}

uint32 getTxConfirmedPkts(HaloUdpStats *stats)
{
    return stats->txConfirmedPkts;
}

uint32 updateTxConfirmedPkts(HaloUdpStats *stats, uint32 pkts)
{
    stats->txConfirmedPkts += pkts;

    return stats->txConfirmedPkts;
}

uint32 getTxDroppedPkts(HaloUdpStats *stats)
{
    return stats->txDroppedPkts;
}

uint32 updateTxDroppedPkts(HaloUdpStats *stats, uint32 pkts)
{
    stats->txDroppedPkts += pkts;

    return stats->txDroppedPkts;
}

uint32 getTxAcks(HaloUdpStats *stats)
{
    return stats->txAcks;
}

uint32 updateTxAcks(HaloUdpStats *stats, uint32 acks)
{
    stats->txAcks += acks;

    return stats->txAcks;
}

uint32 getTxDataPkts(HaloUdpStats *stats)
{
    return stats->txDataPkts;
}

uint32 updateTxDataPkts(HaloUdpStats *stats, uint32 pkts)
{
    stats->txDataPkts += pkts;

    return stats->txDataPkts;
}

uint32 getTxDataBytes(HaloUdpStats *stats)
{
    return stats->txDataBytes;
}

uint32 updateTxDataBytes(HaloUdpStats *stats, uint32 bytes)
{
    stats->txDataBytes += bytes;

    return stats->txDataBytes;
}




uint32 getRxGoodBytes(HaloUdpStats *stats)
{
    return stats->rxGoodBytes;
}

uint32 updateRxGoodBytes(HaloUdpStats *stats, uint32 bytes)
{
    stats->rxGoodBytes += bytes;

    return stats->rxGoodBytes;
}

uint32 getRxGoodPkts(HaloUdpStats *stats)
{
    return stats->rxGoodPkts;
}

uint32 updateRxGoodPkts(HaloUdpStats *stats, uint32 pkts)
{
    stats->rxGoodPkts += pkts;

    return stats->rxGoodPkts;
}

uint32 getRxBadCrcPkts(HaloUdpStats *stats)
{
    return stats->rxBadCrcPkts;
}

uint32 updateRxBadCrcPkts(HaloUdpStats *stats, uint32 pkts)
{
    stats->rxBadCrcPkts += pkts;

    return stats->rxBadCrcPkts;
}

uint32 getRxDuplicatePkts(HaloUdpStats *stats)
{
    return stats->rxDuplicatePkts;
}

uint32 updateRxDuplicatePkts(HaloUdpStats *stats, uint32 pkts)
{
    stats->rxDuplicatePkts += pkts;

    return stats->rxDuplicatePkts;
}

uint32 getRxRuntPkts(HaloUdpStats *stats)
{
    return stats->rxRuntPkts;
}

uint32 updateRxRuntPkts(HaloUdpStats *stats, uint32 pkts)
{
    stats->rxRuntPkts += pkts;

    return stats->rxRuntPkts;
}

uint32 getRxInvalidPayloadPkts(HaloUdpStats *stats)
{
    return stats->rxInvalidPayloadPkts;
}

uint32 updateRxInvalidPayloadPkts(HaloUdpStats *stats, uint32 pkts)
{
    stats->rxInvalidPayloadPkts += pkts;

    return stats->rxInvalidPayloadPkts;
}

uint32 getRxAcks(HaloUdpStats *stats)
{
    return stats->rxAcks;
}

uint32 updateRxAcks(HaloUdpStats *stats, uint32 acks)
{
    stats->rxAcks += acks;

    return stats->rxAcks;
}

uint32 getRxDataPkts(HaloUdpStats *stats)
{
    return stats->rxDataPkts;
}

uint32 updateRxDataPkts(HaloUdpStats *stats, uint32 pkts)
{
    stats->rxDataPkts += pkts;

    return stats->rxDataPkts;
}

uint32 getRxDataBytes(HaloUdpStats *stats)
{
    return stats->rxDataBytes;
}

uint32 updateRxDataBytes(HaloUdpStats *stats, uint32 bytes)
{
    stats->rxDataBytes += bytes;

    return stats->rxDataBytes;
}
