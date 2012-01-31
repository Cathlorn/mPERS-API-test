#include "server_tests.h"
#include "client_test_impl.h"

const TestItem serverPacketTests[] =
{
    {"Send Dynamic Vitals Msg", sendDynamicVitalsPkt},
    {"Send Panic Msg", sendPanicMsgPkt},
    {"Send Fall Msg", sendFallMsgPkt},
    {"Send Operator Acknowledgement Msg", sendOperatorAckMsgPkt},
    {"Send Invalid Msg", sendInvalidMsgPkt},
    {"Listen for Msgs", sendFallMsgPkt},
};

TestSuite serverPacketTesting =
{
    .testSuiteDescription = "MPERS Server Design Verification Tests",
    .testItems = serverPacketTests,
    .testItemCount = (sizeof(serverPacketTests)/sizeof(TestItem)),
};
