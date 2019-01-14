#include <stdio.h>
int main(void)
{
	FILE *fp1=fopen("20khz.pcm","r");
	FILE *fp2=fopen("20khz.pro.pcm","w");
	if(fp1==NULL || fp2==NULL)
	{
		printf("failed to open file.\n");
		return -1;
	}
	while(!feof(fp1))
	{
		short temp1=0,temp2=0;
		unsigned char buffer[2];
		if(fread(buffer,sizeof(buffer),1,fp1)!=1)
		{
			printf("read fp1 error\n");
			break;
		}
		temp1=buffer[1]<<8|buffer[0];
		if(temp1!=0 && abs(temp1*8)<32767 )
		{
			temp2=temp1*8;
			printf("%d -> %d \n",temp1,temp2);
		}else{
			temp2=temp1;
			printf("keep old: %d \n",temp1);
		}
		buffer[1]=(temp2&0xFF00)>>8;
		buffer[0]=(temp2&0xFF);
		if(fwrite(buffer,sizeof(buffer),1,fp2)!=1)
		{
			printf("failed to write fp2!\n");
		}
	}
	fclose(fp1);
	fclose(fp2);
	printf("done\n");
	return 0;
}
