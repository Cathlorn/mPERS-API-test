#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <getopt.h>
#include <assert.h>

#include "test_mode.h"
#include "General/types.h"

typedef struct
{
  uint8 actAsServer;
  char hostname[1024];
  char port[1024];
  uint8 debug;
}
ProgramInitParams;

#define PROGRAM_INIT_PARAMS_INIT(a,b,c,d) { \
  .actAsServer = a, \
  .hostname    = b, \
  .port        = c, \
  .debug       = d \
}

int processArguments ( int argc, char **argv, ProgramInitParams *initParams )
{
    int index;
    int c;

    assert(initParams);
    opterr = 0;

    while ( ( c = getopt ( argc, argv, "sdh::p::" ) ) != -1 )
    {
        switch ( c )
        {

        case 'h':

            if ( optarg )
            {
                strcpy(initParams->hostname,optarg);
            }

            break;

        case 'p':

            if ( optarg )
            {
                strcpy(initParams->port,optarg);
            }

            break;

        case 's':
            initParams->actAsServer = 1;
            break;

        case 'd':
            initParams->debug = 1;
            break;

        case '?':

            if ( ( optopt == 'h' ) || ( optopt == 'p' ) )
                fprintf ( stderr, "Option -%c requires an argument.\n", optopt );
            else if ( isprint ( optopt ) )
                fprintf ( stderr, "Unknown option `-%c'.\n", optopt );
            else
                fprintf ( stderr,
                          "Unknown option character `\\x%x'.\n",
                          optopt );

            return 1;

        default:
            abort ();

        }
    }

    for ( index = optind; index < argc; index++ )
        printf ( "Non-option argument %s\n", argv[index] );

    return 0;

}

int main ( int argc, char *argv[] )
{
    int success = FAIL;

    ProgramInitParams initParams = PROGRAM_INIT_PARAMS_INIT(0, "localhost", "53778", 0);

    processArguments(argc, argv, &initParams);

    if(run_test(initParams.debug, initParams.port))
    {
        success = SUCCESS;
    }
    else
    {
        success = FAIL;
    }

    return success;
}
