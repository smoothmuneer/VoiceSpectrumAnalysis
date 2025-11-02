/* Simple ALSA capture + FFTW demo.
 * Uses headers in src/alsa/include and src/fftw (adjust -I flags accordingly).
 * Captures a short buffer (FRAME_COUNT frames) and prints the magnitude of the first bins.
 */
#include "alsa/include/asoundlib-head.h"
#include "alsa/include/asoundlib-tail.h"
#include "fftw/api/fftw3.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#define SAMPLE_RATE 44100
#define CHANNELS 1
#define FRAME_COUNT 1024  /* frames per read and FFT size */

int main(void)
{
    int err;
    snd_pcm_t *capture_handle = NULL;
    snd_pcm_hw_params_t *hw_params = NULL;
    int16_t *buffer = NULL;
    double *in = NULL;
    fftw_complex *out = NULL;
    fftw_plan plan = NULL;
    int i;

    /* Open default capture device */
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
    if (!buffer) { fprintf(stderr, "malloc failed\n"); goto cleanup; }

    /* Read one block */
    snd_pcm_prepare(capture_handle);
    err = snd_pcm_readi(capture_handle, buffer, FRAME_COUNT);
    if (err == -EPIPE) {
        snd_pcm_prepare(capture_handle);
        fprintf(stderr, "Overrun occurred\n");
        goto cleanup;
    } else if (err < 0) {
        fprintf(stderr, "Read error: %s\n", snd_strerror(err));
        goto cleanup;
    } else if (err != FRAME_COUNT) {
        fprintf(stderr, "Short read, frames read: %d\n", err);
    }

    /* Prepare FFTW buffers */
    in  = fftw_malloc(sizeof(double) * FRAME_COUNT);
    out = fftw_malloc(sizeof(fftw_complex) * (FRAME_COUNT/2 + 1));
    if (!in || !out) { fprintf(stderr, "fftw malloc failed\n"); goto cleanup; }

    /* convert int16 -> double [-1..1] */
    for (i = 0; i < FRAME_COUNT; ++i) {
        in[i] = buffer[i] / 32768.0;
    }

    plan = fftw_plan_dft_r2c_1d(FRAME_COUNT, in, out, FFTW_ESTIMATE);
    fftw_execute(plan);

    /* print first few bins magnitudes */
    printf("Captured %d frames -> FFT magnitudes (first 10 bins):\n", FRAME_COUNT);
    for (i = 0; i < 10 && i <= FRAME_COUNT/2; ++i) {
        double re = out[i][0];
        double im = out[i][1];
        double mag = sqrt(re*re + im*im);
        printf("bin %2d: %8.5f\n", i, mag);
    }

cleanup:
    if (plan) fftw_destroy_plan(plan);
    if (out) fftw_free(out);
    if (in) fftw_free(in);
    if (buffer) free(buffer);
    if (capture_handle) snd_pcm_close(capture_handle);
    return 0;
}