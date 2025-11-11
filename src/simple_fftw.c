#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifdef USE_FFTW
#include <fftw3.h>
#endif

/* If FFTW is not available, use a tiny internal real->half DFT */
#ifndef USE_FFTW
static void real_dft_half(int N, const double *in, double *out_re, double *out_im)
{
    for (int k = 0; k <= N/2; ++k) {
        double re = 0.0, im = 0.0;
        for (int n = 0; n < N; ++n) {
            double angle = -2.0 * M_PI * k * n / N;
            re += in[n] * cos(angle);
            im += in[n] * sin(angle);
        }
        out_re[k] = re;
        out_im[k] = im;
    }
}
#endif

int main(void)
{
    const int N = 8;
    double in[N];
    int i;

    for (i = 0; i < N; ++i) in[i] = sin(2.0*M_PI * i / N);

#ifdef USE_FFTW
    fftw_complex *out = fftw_malloc(sizeof(fftw_complex) * (N/2 + 1));
    if (!out) { fprintf(stderr, "fftw_malloc failed\n"); return 1; }
    fftw_plan p = fftw_plan_dft_r2c_1d(N, in, out, FFTW_ESTIMATE);
    fftw_execute(p);
    printf("FFT result (magnitude) for N=%d (FFTW):\n", N);
    for (i = 0; i <= N/2; ++i) {
        double re = out[i][0];
        double im = out[i][1];
        double mag = sqrt(re*re + im*im);
        printf("bin %2d: %8.4f\n", i, mag);
    }
    fftw_destroy_plan(p);
    fftw_free(out);
#else
    double out_re[N/2 + 1];
    double out_im[N/2 + 1];
    real_dft_half(N, in, out_re, out_im);
    printf("DFT magnitudes (N=%d, internal DFT):\n", N);
    for (i = 0; i <= N/2; ++i) {
        double mag = sqrt(out_re[i]*out_re[i] + out_im[i]*out_im[i]);
        printf("bin %2d: %8.4f\n", i, mag);
    }
#endif

    return 0;
}