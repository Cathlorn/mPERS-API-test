#ifndef CLIENT_MODE_H
#define CLIENT_MODE_H

#include "UdpLib/halo_udp_comm.h"

void run_client(unsigned char debug, char *hostname, char *port, int testNumber, char *testArgs, HaloUdpCommDbg dbgParams);

#endif //CLIENT_MODE_H
