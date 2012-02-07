#ifndef SERVER_MODE_H
#define SERVER_MODE_H

#include "UdpLib/halo_udp_comm.h"

int run_server(unsigned char debug, char *port, int testNumber, char *testArgs, HaloUdpCommDbg dbgParams);

#endif //SERVER_MODE_H
