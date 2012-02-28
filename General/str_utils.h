#ifndef STR_UTILS_H
#define STR_UTILS_H

int findStrMatch( char *matchString, char *valStr);
int strMatchFound( char *matchString, char *valStr);
int isSeparator(char *separators, char c);
int getNextSubArgument(const char * argumentString, int offset, char * separators, char * dstString);

int isHexChar(char c);
void readHexChar(char * hexString, unsigned char *c);
void writeHexChar(unsigned char c, char * outputString);
void shift_left_text_buffer(char *buf, int bufMaxLen, char *data, int len);
void shift_right_text_buffer(char *buf, int bufMaxLen, char *data, int len);

#endif //STR_UTILS_H
