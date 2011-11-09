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
        testSuite->testItems[i].testCall();
    }
    else
    {
        printf("Invalid Menu Choice!\n");
    }
}
