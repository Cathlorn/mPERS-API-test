#include "udp_tests.h"
#include "test_impl.h"

const TestItem udpProtocolTests[] =
{
    {"UDP Bidirectional Comm Test", commTest},
    {"UDP ReOpening Socket Test", reOpenTest},
    {"UDP Bad IP Write Test", badAddrWriteTest},
    {"UDP Read No Data Test", noDataAvailableTest},
    {"UDP Invalid Network Config Test", badNetworkConfigTest},
};

TestSuite udpProtocolTesting =
{
    .testSuiteDescription = "UDP Protocol Tests",
    .testItems = udpProtocolTests,
    .testItemCount = (sizeof(udpProtocolTests)/sizeof(TestItem)),
};
