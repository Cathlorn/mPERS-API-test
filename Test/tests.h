#ifndef TESTS_H
#define TESTS_H

typedef struct
{
    int testNumber;
    char *testArgs;
    int testResult;
}
TestInfo;

#define TEST_INFO_INIT(a,b) { \
 .testNumber  = a, \
 .testArgs = b, \
 .testResult = -1, \
}

typedef int ( *TestFunction ) ( void *args );

typedef struct {
    const char *testDescription;
    TestFunction testCall;
}
TestItem;

typedef struct {
    const char *testSuiteDescription;
    const TestItem *testItems;
    int testItemCount;
}
TestSuite;

void printTestItemMenu(TestSuite *testSuite);
void getTestItemChoice(TestSuite *testSuite);

void showTestItems(TestSuite *testSuite);
int runTestItem(TestSuite *testSuite, unsigned int testIndex, void *args);
int runAllTestItems(TestSuite *testSuite);

#endif //TESTS_H
