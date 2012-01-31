//MyHalo UDP Protocol Statistics Tools

#ifndef HALO_UDP_STATS_H
#define HALO_UDP_STATS_H

#include "types.h"

typedef struct
{
    //Tx Stats
	uint32 txBytes;
	uint32 txConfirmedBytes;
	uint32 txPkts;
	uint32 txConfirmedPkts;
	uint32 txDroppedPkts;
	uint32 txAcks;
	uint32 txDataPkts;
	uint32 txDataBytes;

    //Rx Stats
	uint32 rxGoodBytes;
	uint32 rxGoodPkts;
	uint32 rxBadCrcPkts;
	uint32 rxDuplicatePkts;
	uint32 rxRuntPkts;
	uint32 rxInvalidPayloadPkts;
	uint32 rxAcks;
	uint32 rxDataPkts;
	uint32 rxDataBytes;
}
HaloUdpStats;

#define HALO_UDP_STATS_INIT() { \
                                .txBytes = 0, \
                                .txConfirmedBytes = 0, \
                                .txPkts = 0, \
                                .txConfirmedPkts = 0, \
                                .txDroppedPkts = 0, \
                                .txAcks = 0, \
                                .txDataPkts = 0, \
                                .txDataBytes = 0, \
                                .rxGoodBytes = 0, \
                                .rxGoodPkts = 0, \
                                .rxBadCrcPkts = 0, \
                                .rxDuplicatePkts = 0, \
                                .rxInvalidPayloadPkts = 0, \
                                .rxAcks = 0, \
                                .rxDataPkts = 0, \
                                .rxDataBytes = 0, \
                          }

#define HALO_UDP_STATS_CLR(p) { \
                                p->txBytes = 0; \
                                p->txConfirmedBytes = 0; \
                                p->txPkts = 0; \
                                p->txConfirmedPkts = 0; \
                                p->txDroppedPkts = 0; \
                                p->txAcks = 0; \
                                p->txDataPkts = 0; \
                                p->txDataBytes = 0; \
                                p->rxGoodBytes = 0; \
                                p->rxGoodPkts = 0; \
                                p->rxBadCrcPkts = 0; \
                                p->rxDuplicatePkts = 0; \
                                p->rxInvalidPayloadPkts = 0; \
                                p->rxAcks = 0; \
                                p->rxDataPkts = 0; \
                                p->rxDataBytes = 0; \
                          }

void resetHaloUdpStats(HaloUdpStats *stats);

//Getters/Setters
uint32 getTxBytes(HaloUdpStats *stats);
uint32 updateTxBytes(HaloUdpStats *stats, uint32 bytes);

uint32 getTxConfirmedBytes(HaloUdpStats *stats);
uint32 updateTxConfirmedBytes(HaloUdpStats *stats, uint32 bytes);

uint32 getTxPkts(HaloUdpStats *stats);
uint32 updateTxPkts(HaloUdpStats *stats, uint32 pkts);

uint32 getTxConfirmedPkts(HaloUdpStats *stats);
uint32 updateTxConfirmedPkts(HaloUdpStats *stats, uint32 pkts);

uint32 getTxDroppedPkts(HaloUdpStats *stats);
uint32 updateTxDroppedPkts(HaloUdpStats *stats, uint32 pkts);

uint32 getTxAcks(HaloUdpStats *stats);
uint32 updateTxAcks(HaloUdpStats *stats, uint32 acks);

uint32 getTxDataPkts(HaloUdpStats *stats);
uint32 updateTxDataPkts(HaloUdpStats *stats, uint32 pkts);

uint32 getTxDataBytes(HaloUdpStats *stats);
uint32 updateTxDataBytes(HaloUdpStats *stats, uint32 bytes);



uint32 getRxGoodBytes(HaloUdpStats *stats);
uint32 updateRxGoodBytes(HaloUdpStats *stats, uint32 bytes);

uint32 getRxGoodPkts(HaloUdpStats *stats);
uint32 updateRxGoodPkts(HaloUdpStats *stats, uint32 pkts);

uint32 getRxBadCrcPkts(HaloUdpStats *stats);
uint32 updateRxBadCrcPkts(HaloUdpStats *stats, uint32 pkts);

uint32 getRxDuplicatePkts(HaloUdpStats *stats);
uint32 updateRxDuplicatePkts(HaloUdpStats *stats, uint32 pkts);

uint32 getRxRuntPkts(HaloUdpStats *stats);
uint32 updateRxRuntPkts(HaloUdpStats *stats, uint32 pkts);

uint32 getRxInvalidPayloadPkts(HaloUdpStats *stats);
uint32 updateRxInvalidPayloadPkts(HaloUdpStats *stats, uint32 pkts);

uint32 getRxAcks(HaloUdpStats *stats);
uint32 updateRxAcks(HaloUdpStats *stats, uint32 acks);

uint32 getRxDataPkts(HaloUdpStats *stats);
uint32 updateRxDataPkts(HaloUdpStats *stats, uint32 pkts);

uint32 getRxDataBytes(HaloUdpStats *stats);
uint32 updateRxDataBytes(HaloUdpStats *stats, uint32 bytes);

#endif //HALO_UDP_STATS_H
