#include "halo_udp_tests.h"
#include "test_impl.h"

const TestItem udpProtocolTests[] =
{
    {"HaloUDP Rx Valid Pkt Test", goodPktTest},
    {"HaloUDP Rx Duplicate Pkt Test", duplicatePktTest},
    {"HaloUDP Rx Out of Sequence Pkt Test", rxOutOfSeqTest},
    {"HaloUDP Rx Runt Test", runtTest},
    {"HaloUDP Rx Corrupted Pkt Test", badCrcTest},
    {"HaloUDP Rx Invalid Payload Length Pkt Test", rxInvalidPayloadLengthTest},
    {"HaloUDP Tx Comm Success Test", txGoodTest},
    {"HaloUDP Tx Comm Failure Test", txDropTest},
    {"HaloUDP Tx Beyond Sequence Number Capacity", txSeqNumBeyondSeqNumRange},
    {"HaloUDP Rx Beyond Sequence Number Capacity", rxSeqNumBeyondSeqNumRange},
    {"HaloUDP Tx Session Control Test", txSessionControlTest},
    {"HaloUDP Rx Session Control Test", rxSessionControlTest},
};

TestSuite myHaloUdpProtocolTesting =
{
    .testSuiteDescription = "myHalo UDP Protocol Tests",
    .testItems = udpProtocolTests,
    .testItemCount = (sizeof(udpProtocolTests)/sizeof(TestItem)),
};
