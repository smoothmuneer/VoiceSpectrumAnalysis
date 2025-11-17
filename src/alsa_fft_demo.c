#define _POSIX_C_SOURCE 200809L

/* ALSA capture + FFT demo (optional FFTW). Uses system ALSA headers. */
#include <alsa/asoundlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <alloca.h>

#ifdef USE_FFTW
#include <fftw3.h>
#endif

#define SAMPLE_RATE 44100
#define CHANNELS 1
#define FRAME_COUNT 1024
#define M_PI 3.14159265358979323846

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
    int err;
    snd_pcm_t *capture_handle = NULL;
    snd_pcm_hw_params_t *hw_params = NULL;
    int16_t *buffer = NULL;
    double *in = NULL;

#ifdef USE_FFTW
    fftw_complex *out = NULL;
    fftw_plan plan = NULL;
#else
    double *out_re = NULL;
    double *out_im = NULL;
#endif

    if ((err = snd_pcm_open(&capture_handle, "default", SND_PCM_STREAM_CAPTURE, 0)) < 0) {
        fprintf(stderr, "Cannot open audio device: %s\n", snd_strerror(err));
        return 1;
    }

    snd_pcm_hw_params_alloca(&hw_params);
    snd_pcm_hw_params_any(capture_handle, hw_params);
    snd_pcm_hw_params_set_access(capture_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(capture_handle, hw_params, SND_PCM_FORMAT_S16_LE);
    snd_pcm_hw_params_set_channels(capture_handle, hw_params, CHANNELS);
    unsigned int rate = SAMPLE_RATE;
    snd_pcm_hw_params_set_rate_near(capture_handle, hw_params, &rate, 0);
    snd_pcm_hw_params(capture_handle, hw_params);

    buffer = malloc(sizeof(int16_t) * FRAME_COUNT * CHANNELS);
    in = malloc(sizeof(double) * FRAME_COUNT);
#ifdef USE_FFTW
    out = fftw_malloc(sizeof(fftw_complex) * (FRAME_COUNT/2 + 1));
    if (!buffer || !in || !out) { fprintf(stderr, "malloc failed\n"); goto cleanup; }
    plan = fftw_plan_dft_r2c_1d(FRAME_COUNT, in, out, FFTW_ESTIMATE);
#else
    out_re = malloc(sizeof(double) * (FRAME_COUNT/2 + 1));
    out_im = malloc(sizeof(double) * (FRAME_COUNT/2 + 1));
    if (!buffer || !in || !out_re || !out_im) { fprintf(stderr, "malloc failed\n"); goto cleanup; }
#endif

    snd_pcm_prepare(capture_handle);
    err = snd_pcm_readi(capture_handle, buffer, FRAME_COUNT);
    if (err == -EPIPE) {
        snd_pcm_prepare(capture_handle);
        fprintf(stderr, "Overrun occurred\n");
        goto cleanup;
    } else if (err < 0) {
        fprintf(stderr, "Read error: %s\n", snd_strerror(err));
        goto cleanup;
    }

    for (int i = 0; i < FRAME_COUNT; ++i) in[i] = buffer[i] / 32768.0;

#ifdef USE_FFTW
    fftw_execute(plan);
    printf("Captured %d frames -> FFT (first 10 bins):\n", FRAME_COUNT);
    for (int i = 0; i < 10 && i <= FRAME_COUNT/2; ++i) {
        double re = out[i][0], im = out[i][1];
        printf("bin %2d: %8.5f\n", i, sqrt(re*re + im*im));
    }
#else
    real_dft_half(FRAME_COUNT, in, out_re, out_im);
    printf("Captured %d frames -> DFT (first 10 bins):\n", FRAME_COUNT);
    for (int i = 0; i < 10 && i <= FRAME_COUNT/2; ++i) {
        printf("bin %2d: %8.5f\n", i, sqrt(out_re[i]*out_re[i] + out_im[i]*out_im[i]));
    }
#endif

cleanup:
#ifdef USE_FFTW
    if (plan) fftw_destroy_plan(plan);
    if (out) fftw_free(out);
#else
    if (out_re) free(out_re);
    if (out_im) free(out_im);
#endif
    if (in) free(in);
    if (buffer) free(buffer);
    if (capture_handle) snd_pcm_close(capture_handle);
    return 0;
}
