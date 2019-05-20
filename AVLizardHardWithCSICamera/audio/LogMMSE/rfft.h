#ifndef _RFFT_H
#define _RFFT_H

#ifdef __cplusplus
extern "C"{
#endif

#define M_PI        3.14159265358979323846
#define M_SQRT2     1.41421356237309504880

void rfft(float *x, int n, int m);

#ifdef __cplusplus
};
#endif

#endif
