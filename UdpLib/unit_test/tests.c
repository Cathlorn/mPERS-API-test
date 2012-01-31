#include <stdio.h>

#include "tests.h"

void printTestItemMenu(TestSuite *testSuite)
{
    int i;

    printf("\n\n----------------------------------------\n");
    printf("%s\n", testSuite->testSuiteDescription);
    printf("----------------------------------------\n\n\n");
    for(i = 0; i < testSuite->testItemCount; i++)
    {
        printf("%d. %s\n", i+1, testSuite->testItems[i].testDescription);
    }

    printf("Enter the Test you would like to do: ");
}

void getTestItemChoice(TestSuite *testSuite)
{
    int i;

    scanf("%d", &i);
    i--;

    if((i >= 0)&&(i < testSuite->testItemCount))
    {
        //Call the test function
        testSuite->testItems[i].testCall(NULL);
    }
    else
    {
        printf("Invalid Menu Choice!\n");
    }
}

void showTestItems(TestSuite *testSuite)
{
    int i;
    for(i = 0; i < testSuite->testItemCount; i++)
    {
        printf("%4d. %s\n", i+1, testSuite->testItems[i].testDescription);
    }
}

int runTestItem(TestSuite *testSuite, unsigned int testIndex, void *args)
{
    int passed = 1;

    if(testIndex < testSuite->testItemCount)
    {
        //Call the test function
        passed = testSuite->testItems[testIndex].testCall(args);
    }

    return passed;
}

int runAllTestItems(TestSuite *testSuite)
{
    int passed = 1;
    int i;

    for(i = 0; i < testSuite->testItemCount; i++)
    {
        passed &= runTestItem(testSuite, i, NULL);
    }

    return passed;
}
