#ifndef UBLOX_STR_UTILS_H
#define UBLOX_STR_UTILS_H

int decodeUbloxHexString(char *hexString, int len, unsigned char *data, int max_len);
void createUbloxHexString(unsigned char *data, int len, char *outputString, int outputStringMaxLen);
int ParseIPString(char *ipString);
void GetIpString(int ip, char *outputString, int maxOutputStringLen);

#endif //UBLOX_STR_UTILS_H
