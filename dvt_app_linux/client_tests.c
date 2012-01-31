#include "client_tests.h"
#include "client_test_impl.h"

const TestItem packetTests[] =
{
    {"Send Dynamic Vitals Msg", sendDynamicVitalsPkt},
    {"Send Panic Msg", sendPanicMsgPkt},
    {"Send Fall Msg", sendFallMsgPkt},
    {"Send Operator Acknowledgement Msg", sendOperatorAckMsgPkt},
    {"Send Invalid Msg", sendInvalidMsgPkt},
    {"Listen for Msgs", sendFallMsgPkt},
};

TestSuite packetTesting =
{
    .testSuiteDescription = "MPERS Client Design Verification Tests",
    .testItems = packetTests,
    .testItemCount = (sizeof(packetTests)/sizeof(TestItem)),
};
