#include "udp_tests.h"
#include "test_impl.h"

const TestItem udpProtocolTests[] =
{
    {"UDP Bidirectional Comm Test", commTest},
    {"UDP ReOpening Socket Test", reOpenTest},
};

TestSuite udpProtocolTesting =
{
    .testSuiteDescription = "UDP Protocol Tests",
    .testItems = udpProtocolTests,
    .testItemCount = (sizeof(udpProtocolTests)/sizeof(TestItem)),
};
