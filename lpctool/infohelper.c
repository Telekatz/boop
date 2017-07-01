#include <stdio.h>

#include "infohelper.h"

char verbosity;

int printError(char *err)
{
	printf("ERROR: %s\n",err);
	return 0;
}

int printErrorS(char *err, char *xtra)
{
	if(xtra)
		printf("ERROR: %s %s\n", err, xtra);
	else
		printf("ERROR: %s\n", err);
	return 0;
}

int printInfoS(unsigned char level, char *msg, char *xtra)
{
	if(level <= verbosity)
	{
	if(xtra)
		printf("Info : %s %s\n", msg, xtra);
	else
		printf("Info : %s\n", msg);
		return 1;
	}
	return 0;
}

int printInfoH(unsigned char level, char *msg, unsigned long val)
{
	if(level <= verbosity)
	{
		printf("Info : %s 0x%08X\n", msg, val);
		return 1;
	}
	return 0;
}

const char heartbeat[] = { ".oOo" };

int printProgress(unsigned char level, char *msg, float cval, float maxval)
{
	unsigned int cur, cnt;

	static char beat = 0;

	if(level <= verbosity)
	{
		cur = (35 / maxval) * cval;
	
		cnt = 35-cur;
	
		printf("Progress: [");
	
		while(cur--)
			printf("*");

		if(cnt)
			printf("%c",heartbeat[beat]);
		else
			printf("*");
	
		while(cnt--)
			printf(" ");
	
		printf("\b]\r");
		fflush(stdout);
		beat++;
		beat &= 0x03;
	}
}

