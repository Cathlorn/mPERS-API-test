#include "test_mode.h"

#define SUCCESS  0
#define FAIL    -1

int main ( int argc, char *argv[] )
{
    int success = FAIL;

    if(udp_test())
    {
        success = SUCCESS;
    }
    else
    {
        success = FAIL;
    }

    return success;
}
