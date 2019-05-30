#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#define Y_SIZE	(1920*1080)
#define UV_SIZE	(1920/2*1080/2)*2
int main(int argc,char **argv)
{
	char *fileYUV="in.yuv";
	char *fileYVU="out.yvu";
	int fdIn,fdOut;
	int i,offset;
	fdIn=open("in.yuv",O_RDONLY);
	if(fdIn<0)
	{
		printf("failed to open in.yuv!\n");
		return -1;
	}
	fdOut=open("out.yvu",O_WRONLY|O_CREAT);
	if(fdOut<0)
	{
		printf("failed to open out.yvu!\n");
		return -1;
	}
	char *buffer=malloc(Y_SIZE);
	if(buffer==NULL)
	{
		printf("failed to malloc()!\n");
		return -1;
	}
	memset(buffer,0,Y_SIZE);
	read(fdIn,buffer,Y_SIZE);
	write(fdOut,buffer,Y_SIZE);

	memset(buffer,0,Y_SIZE);
	read(fdIn,buffer,UV_SIZE);
	for(i=0,offset=0;i<UV_SIZE/2;i++,offset+=2)
	{
		char tmp;
		tmp=buffer[offset+1];
		buffer[offset+1]=buffer[offset];
		buffer[offset]=tmp;	
	}	
	write(fdOut,buffer,UV_SIZE);
	
	close(fdIn);
	close(fdOut);
	free(buffer);
	printf("done!\n");
	return 0;
}
