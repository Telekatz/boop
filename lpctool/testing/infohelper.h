#ifndef TOOL_INFOHELPER
#define TOOL_INFOHELPER

extern char verbosity;

int printError(char *err);
int printErrorS(char *err, char *xtra);
int printInfoS(unsigned char level, char *msg, char *xtra);
int printInfoC(unsigned char level, char *msg, char xtra);
int printInfoH(unsigned char level, char *msg, unsigned long val);
int printInfoU(unsigned char level, char *msg, unsigned long val);
int printProgress(unsigned char level, char *msg, float cval, float maxval);

#endif
