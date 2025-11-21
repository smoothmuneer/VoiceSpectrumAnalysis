# VoiceSpectrumAnalysis

1. Go to the /src-folder and compile with alsa-record-example.c with: 

gcc -std=c11 -O2 -Wall alsa-record-example.c -o alsa-record-example -lasound



2. You can play the audio on cli with: 

./alsa-record-example default audio.raw



3. To analyse the audio.raw file, compile first the alsa_fft_sample.c:

gcc -std=c11 -O2 -Wall alsa_fft_sample.c -o alsa_fft_sample -lm



4. Execute the the compiled file with the audio.raw as a parameter on the cli:

./alsa_fft_sample audio.raw



You can also play the audio with the command:

aplay -f S16_LE -r 44100 -c 2 audio.raw