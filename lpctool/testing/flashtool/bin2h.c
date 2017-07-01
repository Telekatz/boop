#include "stdio.h"
#include "string.h"
#include <fcntl.h>


int main(void)
{
	int fd;
	unsigned long cnt, cnt2;
	unsigned char buf[0x10000];

	fd = open("ramtool.bin", O_RDONLY);

	if(fd)
	{
		cnt = read(fd, buf, 0x10000);
		cnt2 = 0;
		printf("#define tool_len 0x%08lX\n",cnt);
		printf("const unsigned char tool_data[] = {\n\t");
		while(cnt--)
		{
			printf("0x%02X,",buf[cnt2++]);
			if((cnt2 & 0x0F) == 0)
				printf("\n\t");
		}
		printf("\n};\n");
		close(fd);
	}
	return 0;
}
