#include <stdio.h>
#include <string.h>
#include <stdlib.h>
int main(void)
{
    unsigned int errcount=0;
    unsigned int okcount=0;
    FILE *fp=fopen("cap24bit.dat","r");
    if(fp==NULL)
    {
        printf("failed to open file!\n");
        return -1;
    }
    while(!feof(fp))
    {
        //a1,a2,a3,b1,b2,b3,   c1,c2,c3,d1,d2,d3.
        //a=b && c=d
        //c=a+1.
        unsigned char adbytes[12];
        unsigned int a,b,c,d;

        //read 6 bytes.
        if(fread(adbytes,sizeof(adbytes),1,fp)!=1)
        {
            printf("reach end of file\n");
            break;
        }
        a=(int)adbytes[0]<<16|(int)adbytes[1]<<8|(int)adbytes[2]<<0;
        b=(int)adbytes[3]<<16|(int)adbytes[4]<<8|(int)adbytes[5]<<0;
        c=(int)adbytes[6]<<16|(int)adbytes[7]<<8|(int)adbytes[8]<<0;
        d=(int)adbytes[9]<<16|(int)adbytes[10]<<8|(int)adbytes[11]<<0;

        if((a==b) && (c==d) && (a+1==c))
        {
            //printf("a=%d,b=%d,c=%d,d=%d,check okay.\n",a,b,c,d);
            okcount++;
        }else{
            printf("a=%d,b=%d,c=%d,d=%d,check error!!! times=%d\n",a,b,c,d,errcount++);
        }
    }
    fclose(fp);
    printf("check finished:okay times=%d,error times=%d\n",okcount,errcount);
    return 0;
}
