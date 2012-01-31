#ifndef STR_UTILS_H
#define STR_UTILS_H

int findStrMatch( char *matchString, char *valStr);
int strMatchFound( char *matchString, char *valStr);
int isSeparator(char *separators, char c);
int getNextSubArgument(const char * argumentString, int offset, char * separators, char * dstString);

#endif //STR_UTILS_H
