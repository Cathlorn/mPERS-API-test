#ifndef TESTS_H
#define TESTS_H

typedef void ( *TestFunction ) ( void );

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

#endif //TESTS_H
