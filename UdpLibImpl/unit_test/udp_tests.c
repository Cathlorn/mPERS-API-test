#include "udp_tests.h"
#include "test_impl.h"

const TestItem udpProtocolTests[] =
{
    {"UDP Tx Comm Success Test", txGoodTest},
};

TestSuite udpProtocolTesting =
{
    .testSuiteDescription = "UDP Protocol Tests",
    .testItems = udpProtocolTests,
    .testItemCount = (sizeof(udpProtocolTests)/sizeof(TestItem)),
};
