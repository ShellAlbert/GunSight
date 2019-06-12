// myWebRtcTest.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <Windows.h>

#include "signal_processing_library.h"
#include "noise_suppression.h"
#include "gain_control.h"

int NoiseSuppression16_16k(char *szFileIn,char *szFileOut,int nSample,int nMode)
{
	int nRet = 0;
	NsHandle *pNS_inst = NULL;

	FILE *fpIn = NULL;
	FILE *fpOut = NULL;

	int nTime = 0;

	if (16000 != nSample)
		return 1;

	if (0 != WebRtcNs_Create(&pNS_inst))
	{
		printf("Noise_Suppression WebRtcNs_Create err! \n");
		return 1;
	}

	if (0 !=  WebRtcNs_Init(pNS_inst,nSample))
	{
		printf("Noise_Suppression WebRtcNs_Init err! \n");
		return 1;
	}

	if (0 !=  WebRtcNs_set_policy(pNS_inst,nMode))
	{
		printf("Noise_Suppression WebRtcNs_set_policy err! \n");
		return 1;
	}

	fpIn = fopen(szFileIn, "rb");
	if (NULL == fpIn)
	{
		printf("open src file err \n");
		return 1;
	}

	fpOut = fopen(szFileOut, "wb");
	if (NULL == fpOut)
	{
		printf("open out file err! \n");
		return 1;
	}

	short	shInL[160] = {0};
	short	shOutL[160] = {0};
	int		length;
	int		frame = 0;

//	nTime = GetTickCount();

	while(!feof(fpIn))
	{
		memset(shInL, 0, 160*sizeof(short));
		memset(shOutL, 0, 160*sizeof(short));
		length = fread(shInL, sizeof(short), 160, fpIn);

		//将需要降噪的数据以高频和低频传入对应接口，16kHz只需处理低频部分即可
		nRet = WebRtcNs_Process(pNS_inst, shInL, NULL, shOutL, NULL);
		if (nRet != 0)
		{
			printf("WebRtcNs_Process err! \n");
			return 1;
		}
		fwrite(shOutL, sizeof(short), length, fpOut);
		frame++;
	}

//	nTime = GetTickCount() - nTime;
//	printf("n_s user time=%dms, %d frames\n",nTime, frame);
 		printf("n_s %d frames\n", frame);

	WebRtcNs_Free(pNS_inst);
	fclose(fpIn);
	fclose(fpOut);

	return 0;
}

int NoiseSuppression16_48k(char *szFileIn,char *szFileOut,int nSample,int nMode)
{
	int nRet = 0;
	NsHandle *pNS_inst = NULL;
	WebRtcSpl_State16khzTo48khz state_16to48;
	WebRtcSpl_State48khzTo16khz state_48to16;

	FILE *fpIn = NULL;
	FILE *fpOut = NULL;

	int nTime = 0;

	if (48000 != nSample)
		return 1;

	WebRtcSpl_ResetResample16khzTo48khz(&state_16to48);
	WebRtcSpl_ResetResample48khzTo16khz(&state_48to16);

	if (0 != WebRtcNs_Create(&pNS_inst))
	{
		printf("Noise_Suppression WebRtcNs_Create err! \n");
		return 1;
	}

	if (0 !=  WebRtcNs_Init(pNS_inst,16000))
	{
		printf("Noise_Suppression WebRtcNs_Init err! \n");
		return 1;
	}

	if (0 !=  WebRtcNs_set_policy(pNS_inst,nMode))
	{
		printf("Noise_Suppression WebRtcNs_set_policy err! \n");
		return 1;
	}

	fpIn = fopen(szFileIn, "rb");
	if (NULL == fpIn)
	{
		printf("open src file err \n");
		return 1;
	}

	fpOut = fopen(szFileOut, "wb");
	if (NULL == fpOut)
	{
		printf("open out file err! \n");
		return 1;
	}

	int16_t inOutData[480] = {0};
	int32_t tmpBuffer[512] = {0};
	short	shInL[160] = {0};
	short	shOutL[160] = {0};
	int		length;
	int		frame = 0;

//	nTime = GetTickCount();

	while(!feof(fpIn))
	{
		memset(inOutData, 0, 480*sizeof(int16_t));
		memset(tmpBuffer, 0, 512*sizeof(int32_t));		
		memset(shInL, 0, 160*sizeof(short));
		memset(shOutL, 0, 160*sizeof(short));

		//read data from pcm file, 48kHz
		length = fread(inOutData, sizeof(int16_t), 480, fpIn);
		if (0 == length)
			break;
		//resample from 48kHz to 16kHz
		WebRtcSpl_Resample48khzTo16khz(inOutData, shInL, &state_48to16, tmpBuffer);

		//将需要降噪的数据以高频和低频传入对应接口，16kHz只需处理低频部分即可
		nRet = WebRtcNs_Process(pNS_inst, shInL, NULL, shOutL, NULL);
		if (nRet != 0)
		{
			printf("WebRtcNs_Process err! \n");
			return 1;
		}

		//resample from 16kHz to 48kHz
		memset(inOutData, 0, 480*sizeof(int16_t));
		WebRtcSpl_Resample16khzTo48khz(shOutL, inOutData, &state_16to48, tmpBuffer);

		fwrite(inOutData, sizeof(int16_t), length, fpOut);
		frame++;
	}

//	nTime = GetTickCount() - nTime;
//	printf("n_s user time=%dms, %d frames\n",nTime, frame);
 		printf("n_s %d frames\n", frame);
 		
	WebRtcNs_Free(pNS_inst);
	fclose(fpIn);
	fclose(fpOut);

	return 0;
}

int main(int argc, char* argv[])
{
	int sample;
	int mode = 3;
	
  if (argc!=4) 
  {
    printf("usage: %s <noised speech> <denoised speech> <sample rate>\n", argv[0]);
    return 1;
  }
  
  sample = atoi(argv[3]);

  if (sample == 16000)
  {
  	NoiseSuppression16_16k(argv[1], argv[2], sample, mode);
  }
  else if (sample == 48000)
  {
		NoiseSuppression16_48k(argv[1], argv[2], sample, mode);
  }
  else
  {
    printf("Sample rate should be 16000 or 48000.\n");
    return 1;
  }
 	
	return 0;
}

