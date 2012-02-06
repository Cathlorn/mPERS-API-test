#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <getopt.h>
#include <assert.h>

#include "client_mode.h"
#include "server_mode.h"
#include "client_tests.h"
#include "server_tests.h"
#include "types.h"
#include "str_utils.h"

typedef struct
{
    uint8 actAsServer;
    char hostname[1024];
    char port[1024];
    uint8 debug;
    uint8 displayTests;
    int testNumber;
    char testArgumentString[1024];
    HaloUdpCommDbg dbgParams;
}
ProgramInitParams;

#define PROGRAM_INIT_PARAMS_INIT(a,b,c,d) { \
  .actAsServer  = a, \
  .hostname     = b, \
  .port         = c, \
  .debug        = d, \
  .displayTests = 0, \
  .testNumber = -1, \
  .testArgumentString = "", \
  .dbgParams = HALO_UDP_COMM_DBG_INIT(), \
}

int processArguments ( int argc, char **argv, ProgramInitParams *initParams )
{
    int index;
    int c;

    assert(initParams);
    opterr = 0;

    if(argc == 1)
    {
        //Prints the argument descriptions
        printf("%-40s     %40s\n","-h [hostname]",
               "Network host name to connect to in client mode. Name to bind to in server mode.");
        printf("%-40s     %40s\n","-p [port]",
               "Network port to connect to in client mode. Port to bind to in server mode.");
        printf("%-40s     %40s\n","-t [test number]",
                "Test Item to Run. (Just '-t' displays tests)");
        printf("%-40s     %40s\n","-a [arg1],[arg2],...",
                "Test Item Arguments. Arguments for the test should be comma separated.");
        printf("%-40s     %40s\n","-d [badcrc,commfail=[% fail],noack,outseq]",
               "Debug Control flags to test failure conditions.");
        printf("%-40s     %40s\n","-s",
               "Server Mode (without argument, default is client mode.");
        printf("%-40s     %40s\n","-v",
               "Verbose. Give extra details about what is going on.");
    }

    while ( ( c = getopt ( argc, argv, "sva:d:h:p:t::" ) ) != -1 )
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

        case 'd':

            if ( optarg )
            {
                char subArg[1024];
                int currentIndex = 0;
                int foundAnArgument = 0;

                do
                {
                    currentIndex = getNextSubArgument(optarg, currentIndex, ",;\n", subArg);
                    if(currentIndex >= 0)
                    {
                        foundAnArgument = 1;
                        currentIndex++; //skip the separating character
                    }
                    else
                    {
                        foundAnArgument = 0;
                    }

                    if(foundAnArgument)
                    {
                        if(strMatchFound("badcrc", subArg))
                        {
                            printf("Bad CRC enabled\n");
                            initParams->dbgParams.badCrc = 1;
                        }

                        if(strMatchFound("noack", subArg))
                        {
                            printf("No Acknowledgement enabled\n");
                            initParams->dbgParams.neverAck = 1;
                        }

                        if(strMatchFound("outseq", subArg))
                        {
                            printf("Out of Sequence Numbering enabled\n");
                            initParams->dbgParams.outOfSeqTx = 1;
                        }

                        if(strMatchFound("commfail", subArg))
                        {
                            int commArgStartIndex = 0;
                            int percentFail = 0;

                            commArgStartIndex = findStrMatch("commfail=", subArg);

                            if(commArgStartIndex >= 0)
                            {
                                commArgStartIndex += strlen("commfail=");
                                percentFail = atoi(&subArg[commArgStartIndex]);
                            }
                            else
                            {
                                percentFail = 80;
                                printf("Using Default\n");
                            }

                            if((percentFail > 0)&&(percentFail <= 100))
                            {
                                printf("Comm Fail set to (%d %%)\n", percentFail);
                                initParams->dbgParams.spottyRx = percentFail;
                            }
                        }
                    }
                }
                while(foundAnArgument);
            }

            break;

        case 's':
            initParams->actAsServer = 1;
            break;

        case 't':
            if ( optarg )
            {
                initParams->testNumber = atoi(optarg);
            }
            else
            {
                initParams->displayTests = 1;
            }
            break;

        case 'a':
            if ( optarg )
            {
                strcpy(initParams->testArgumentString, optarg);
            }
            else
            {
                //Write an empty string if there are no arguments
                strcpy(initParams->testArgumentString, "");
            }
            break;

        case 'v':
            initParams->debug = 1;
            break;

        case '?':

            if ( ( optopt == 'h' ) || ( optopt == 'p' ) || ( optopt == 'a' ) || ( optopt == 'd' ) )
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
    ProgramInitParams initParams = PROGRAM_INIT_PARAMS_INIT(0, "localhost", "53778", 0);

    processArguments(argc, argv, &initParams);

    if (initParams.displayTests)
    {
        if (initParams.actAsServer)
            showTestItems(&serverPacketTesting);
        else
            showTestItems(&packetTesting);
    }
    else
    {
        if (initParams.actAsServer)
            run_server(initParams.debug, initParams.port, initParams.testNumber,
                       initParams.testArgumentString, initParams.dbgParams);
        else
            run_client(initParams.debug, initParams.hostname, initParams.port,
                       initParams.testNumber, initParams.testArgumentString,
                       initParams.dbgParams);
    }

    return 0;
}
