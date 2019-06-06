#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <signal.h>
#include <arpa/inet.h>
//how to play in rk3399-linux board.
//aplay -D plughw:CARD=realtekrt5651co,DEV=0 -c 2 -r 32000 -f S24_3LE -t raw cap.pcm
static int g_ExitFlag=0;
void gSigInt(int signo)
{
	g_ExitFlag=1;	
}
int main(void)
{
	int ret,i;
	char pcmBuffer[1024];
	int fd=open("/dev/fpga-i2s",O_RDONLY);
	FILE *fp;
	if(fd<0)
	{
		printf("failed to open fpga-i2s.\n");
		return -1;
	}
	fp=fopen("cap.pcm","w+");
	if(fp==NULL)
	{
		printf("failed to open pcm file!\n");
		return -1;
	}
	signal(SIGINT,gSigInt);
	while(!g_ExitFlag)
	{
		ret=read(fd,pcmBuffer,sizeof(pcmBuffer));	
		printf("read %d bytes:\n",ret);
//		fwrite(pcmBuffer,ret,1,fp);
	#if 1 
		for(i=0;i<ret;i++)
		{
			printf("%02x ",pcmBuffer[i]);
		}
		printf("\n");
	#endif
	}
	close(fd);
	fclose(fp);
	printf("done.\n");
	return 0;
}
