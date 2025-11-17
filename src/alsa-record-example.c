#define _POSIX_C_SOURCE 200809L

/* 
  A Minimal Capture Program

  This program opens an audio interface for capture, configures it for
  stereo, 16 bit, 44.1kHz, interleaved conventional read/write
  access. Then its reads a chunk of random data from it, and exits. It
  isn't meant to be a real program.

  From on Paul David's tutorial : http://equalarea.com/paul/alsa-audio.html

  Fixes rate and buffer problems

  sudo apt-get install libasound2-dev
  gcc -o alsa-record-example -lasound alsa-record-example.c && ./alsa-record-example hw:0
*/

#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>
          
int main(int argc, char *argv[])
{
  int i;
  int err;
  char *buffer;
  int buffer_frames = 128;
  unsigned int rate = 44100;
  snd_pcm_t *capture_handle;
  snd_pcm_hw_params_t *hw_params;
  snd_pcm_format_t format = SND_PCM_FORMAT_S16_LE;
  FILE *output_file;

  /* Check command line arguments */
  if (argc < 2) {
    fprintf(stderr, "Usage: %s <device_name> [output_file]\n", argv[0]);
    fprintf(stderr, "Example: %s default audio.raw\n", argv[0]);
    fprintf(stderr, "Example: %s hw:0 audio.raw\n", argv[0]);
    exit(1);
  }

  /* Open output file (default: audio.raw) */
  const char *output_filename = (argc >= 3) ? argv[2] : "audio.raw";
  output_file = fopen(output_filename, "wb");
  if (!output_file) {
    fprintf(stderr, "cannot open output file %s\n", output_filename);
    exit(1);
  }
  fprintf(stdout, "output file opened: %s\n", output_filename);

  if ((err = snd_pcm_open(&capture_handle, argv[1], SND_PCM_STREAM_CAPTURE, 0)) < 0) {
    fprintf(stderr, "cannot open audio device %s (%s)\n", 
             argv[1],
             snd_strerror(err));
    exit(1);
  }

  fprintf(stdout, "audio interface opened\n");
           
  if ((err = snd_pcm_hw_params_malloc(&hw_params)) < 0) {
    fprintf(stderr, "cannot allocate hardware parameter structure (%s)\n",
             snd_strerror(err));
    exit(1);
  }

  fprintf(stdout, "hw_params allocated\n");
                 
  if ((err = snd_pcm_hw_params_any(capture_handle, hw_params)) < 0) {
    fprintf(stderr, "cannot initialize hardware parameter structure (%s)\n",
             snd_strerror(err));
    exit(1);
  }

  fprintf(stdout, "hw_params initialized\n");
    
  if ((err = snd_pcm_hw_params_set_access(capture_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
    fprintf(stderr, "cannot set access type (%s)\n",
             snd_strerror(err));
    exit(1);
  }

  fprintf(stdout, "hw_params access setted\n");
    
  if ((err = snd_pcm_hw_params_set_format(capture_handle, hw_params, format)) < 0) {
    fprintf(stderr, "cannot set sample format (%s)\n",
             snd_strerror(err));
    exit(1);
  }

  fprintf(stdout, "hw_params format setted\n");
    
  if ((err = snd_pcm_hw_params_set_rate_near(capture_handle, hw_params, &rate, 0)) < 0) {
    fprintf(stderr, "cannot set sample rate (%s)\n",
             snd_strerror(err));
    exit(1);
  }
    
  fprintf(stdout, "hw_params rate setted\n");

  if ((err = snd_pcm_hw_params_set_channels(capture_handle, hw_params, 2)) < 0) {
    fprintf(stderr, "cannot set channel count (%s)\n",
             snd_strerror(err));
    exit(1);
  }

  fprintf(stdout, "hw_params channels setted\n");
    
  if ((err = snd_pcm_hw_params(capture_handle, hw_params)) < 0) {
    fprintf(stderr, "cannot set parameters (%s)\n",
             snd_strerror(err));
    exit(1);
  }

  fprintf(stdout, "hw_params setted\n");
    
  snd_pcm_hw_params_free(hw_params);

  fprintf(stdout, "hw_params freed\n");
    
  if ((err = snd_pcm_prepare(capture_handle)) < 0) {
    fprintf(stderr, "cannot prepare audio interface for use (%s)\n",
             snd_strerror(err));
    exit(1);
  }

  fprintf(stdout, "audio interface prepared\n");

  int sample_size = snd_pcm_format_width(format) / 8 * 2; /* 2 channels */
  buffer = malloc(128 * sample_size);

  fprintf(stdout, "buffer allocated\n");

  for (i = 0; i < 10; ++i) {
    if ((err = snd_pcm_readi(capture_handle, buffer, buffer_frames)) != buffer_frames) {
      fprintf(stderr, "read from audio interface failed (%s)\n",
               snd_strerror(err));
      exit(1);
    }
    
    /* Write to file */
    size_t bytes_written = fwrite(buffer, 1, buffer_frames * sample_size, output_file);
    if (bytes_written != buffer_frames * sample_size) {
      fprintf(stderr, "warning: wrote %zu bytes instead of %d\n", bytes_written, buffer_frames * sample_size);
    }
    
    fprintf(stdout, "read %d done, wrote %zu bytes\n", i, bytes_written);
  }

  free(buffer);

  fprintf(stdout, "buffer freed\n");
  
  fclose(output_file);
  fprintf(stdout, "output file closed\n");
    
  snd_pcm_close(capture_handle);
  fprintf(stdout, "audio interface closed\n");

  exit(0);
}
