#ifndef CRC16_H
#define CRC16_H

#include "General/types.h"

extern const uint16 hdlc_init_fcs16;
extern const uint16 hdlc_good_fcs16;

uint16 hdlcFcs16(uint16 fcs, const byte* cp, int len);
void tryFcs16(byte* cp, int len);

#endif //CRC16_H
