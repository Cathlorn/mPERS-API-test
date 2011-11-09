#ifndef TX_BUFFER_H
#define TX_BUFFER_H

//Configuration: Let's you figure out how you want the memory managed
#define USE_STATIC_IMPL 1

#if USE_STATIC_IMPL
#include "tx_buffer_static.h"

#endif

#endif //TX_BUFFER_H


