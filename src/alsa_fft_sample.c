#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <alloca.h>

#ifdef USE_FFTW
#include <fftw3.h>
#endif

#define SAMPLE_RATE 44100
#define CHANNELS 2
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

int main(int argc, char *argv[])
{
    FILE *input_file = NULL;
    int16_t *buffer = NULL;
    double *in = NULL;

#ifdef USE_FFTW
    fftw_complex *out = NULL;
    fftw_plan plan = NULL;
#else
    double *out_re = NULL;
    double *out_im = NULL;
#endif

    /* Check for input file argument */
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <audio_file.raw>\n", argv[0]);
        fprintf(stderr, "Example: %s audio.raw\n", argv[0]);
        return 1;
    }

    /* Open input file */
    input_file = fopen(argv[1], "rb");
    if (!input_file) {
        fprintf(stderr, "Cannot open file: %s\n", argv[1]);
        return 1;
    }
    fprintf(stdout, "Input file opened: %s\n", argv[1]);

    /* Allocate buffers */
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

    /* Read and process audio in chunks */
    int chunk = 0;
    size_t frames_read;
    while ((frames_read = fread(buffer, sizeof(int16_t) * CHANNELS, FRAME_COUNT, input_file)) > 0) {
        printf("\n=== Chunk %d (%zu frames) ===\n", chunk++, frames_read);

        /* Convert int16 to double */
        for (size_t i = 0; i < frames_read; ++i) {
            /* Use first channel only */
            in[i] = buffer[i * CHANNELS] / 32768.0;
        }

        /* Pad with zeros if less than FRAME_COUNT samples */
        for (size_t i = frames_read; i < FRAME_COUNT; ++i) {
            in[i] = 0.0;
        }

#ifdef USE_FFTW
        fftw_execute(plan);
        printf("FFT magnitudes (first 20 bins):\n");
        for (int i = 0; i < 20 && i <= FRAME_COUNT/2; ++i) {
            double re = out[i][0], im = out[i][1];
            double mag = sqrt(re*re + im*im);
            double freq = (double)i * SAMPLE_RATE / FRAME_COUNT;
            printf("bin %3d (%.1f Hz): %10.2f\n", i, freq, mag);
        }
#else
        real_dft_half(FRAME_COUNT, in, out_re, out_im);
        printf("DFT magnitudes (first 20 bins):\n");
        for (int i = 0; i < 20 && i <= FRAME_COUNT/2; ++i) {
            double mag = sqrt(out_re[i]*out_re[i] + out_im[i]*out_im[i]);
            double freq = (double)i * SAMPLE_RATE / FRAME_COUNT;
            printf("bin %3d (%.1f Hz): %10.2f\n", i, freq, mag);
        }
#endif
    }

    printf("\nFile processing completed.\n");

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
    if (input_file) fclose(input_file);
    return 0;
}