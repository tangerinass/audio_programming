#ifndef FFT_
#define FFT_

#include <complex.h>
typedef double complex cplx;

void _fft(cplx buf[], cplx out[], int n, int step);

void fft(cplx buf[], int n);

void show(const char * s, cplx buf[]);

#endif
