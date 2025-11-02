#include "fftw/api/fftw3.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* Very small FFTW example: real -> complex */
int main(void)
{
    const int N = 8;
    double in[N];
    fftw_complex *out;
    fftw_plan p;
    int i;

    /* simple test signal: a 1-cycle sinus over N samples */
    for (i = 0; i < N; ++i) in[i] = sin(2.0*M_PI * i / N);

    out = fftw_malloc(sizeof(fftw_complex) * (N/2 + 1));
    if (!out) {
        fprintf(stderr, "fftw_malloc failed\n");
        return 1;
    }

    p = fftw_plan_dft_r2c_1d(N, in, out, FFTW_ESTIMATE);
    fftw_execute(p);

    printf("FFT result (magnitude) for N=%d:\n", N);
    for (i = 0; i <= N/2; ++i) {
        double re = out[i][0];
        double im = out[i][1];
        double mag = sqrt(re*re + im*im);
        printf("bin %2d: %8.4f\n", i, mag);
    }

    fftw_destroy_plan(p);
    fftw_free(out);
    return 0;
}