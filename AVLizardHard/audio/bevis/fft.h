/* Written by Bevis,  2011-06-10 */

#ifndef _FFT_H_
#define _FFT_H_
#include "common.h"

#define MAX_FFT_SIZE 2048
#define MAX_FFT_TABLE   5

extern Float bevisHann(int m,int HannSize);
extern Bool bevisFFT (Float real[],Float imag[],Int32 nFFT);
extern Bool bevisIFFT(Float real[],Float imag[],Int32 nFFT);

#endif
