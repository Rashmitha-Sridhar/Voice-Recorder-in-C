#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>
#include <time.h>
#include <direct.h>  // For _mkdir()
#include <stdlib.h>

#define SAMPLE_RATE 44100
#define BITS_PER_SAMPLE 16
#define CHANNELS 1
#define RECORDINGS_FOLDER "Recordings"

void createFolder() {
    if (_mkdir(RECORDINGS_FOLDER) == 0) {
        printf("Created folder: %s\n", RECORDINGS_FOLDER);
    }
}

void getUniqueFilename(char *wavFilename) {
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    sprintf(wavFilename, "%s\\recording_%d-%02d-%02d_%02d-%02d-%02d.wav",
            RECORDINGS_FOLDER, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, 
            tm.tm_hour, tm.tm_min, tm.tm_sec);
}

void saveAsWav(const char *filename, WAVEHDR waveHdr) {
    FILE *file = fopen(filename, "wb");
    if (!file) {
        printf("Error creating WAV file!\n");
        return;
    }

    unsigned int chunkSize = 36 + waveHdr.dwBufferLength;
    unsigned short audioFormat = 1;
    unsigned short numChannels = CHANNELS;
    unsigned int sampleRate = SAMPLE_RATE;
    unsigned short bitsPerSample = BITS_PER_SAMPLE;
    unsigned short blockAlign = numChannels * (bitsPerSample / 8);
    unsigned int byteRate = sampleRate * blockAlign;

    fwrite("RIFF", 1, 4, file);
    fwrite(&chunkSize, 4, 1, file);
    fwrite("WAVE", 1, 4, file);
    fwrite("fmt ", 1, 4, file);
    
    unsigned int subchunk1Size = 16;
    fwrite(&subchunk1Size, 4, 1, file);
    fwrite(&audioFormat, 2, 1, file);
    fwrite(&numChannels, 2, 1, file);
    fwrite(&sampleRate, 4, 1, file);
    fwrite(&byteRate, 4, 1, file);
    fwrite(&blockAlign, 2, 1, file);
    fwrite(&bitsPerSample, 2, 1, file);
    
    fwrite("data", 1, 4, file);
    fwrite(&waveHdr.dwBufferLength, 4, 1, file);
    fwrite(waveHdr.lpData, 1, waveHdr.dwBufferLength, file);

    fclose(file);
    printf("WAV file saved: %s\n", filename);
}

int main() {
    HWAVEIN hWaveIn;
    WAVEFORMATEX wfx;
    WAVEHDR waveHdr;
    char *buffer;
    int recordTime;
    char wavFilename[100];

    // Create "Recordings" folder if it doesn't exist
    createFolder();

    // Get recording duration from user
    printf("Enter recording duration in seconds: ");
    scanf("%d", &recordTime);

    // Generate unique filename
    getUniqueFilename(wavFilename);

    wfx.wFormatTag = WAVE_FORMAT_PCM;
    wfx.nChannels = CHANNELS;
    wfx.nSamplesPerSec = SAMPLE_RATE;
    wfx.wBitsPerSample = BITS_PER_SAMPLE;
    wfx.nBlockAlign = (wfx.nChannels * wfx.wBitsPerSample) / 8;
    wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
    wfx.cbSize = 0;

    waveInOpen(&hWaveIn, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL);

    int bufferSize = SAMPLE_RATE * CHANNELS * (BITS_PER_SAMPLE / 8) * recordTime;
    buffer = (char *)malloc(bufferSize);
    if (!buffer) {
        printf("Memory allocation failed!\n");
        return 1;
    }

    waveHdr.lpData = buffer;
    waveHdr.dwBufferLength = bufferSize;
    waveHdr.dwFlags = 0;
    waveInPrepareHeader(hWaveIn, &waveHdr, sizeof(WAVEHDR));
    waveInAddBuffer(hWaveIn, &waveHdr, sizeof(WAVEHDR));

    waveInStart(hWaveIn);
    printf("Recording for %d seconds...\n", recordTime);
    Sleep(recordTime * 1000);  // Wait for the recording duration

    waveInStop(hWaveIn);
    waveInClose(hWaveIn);

    saveAsWav(wavFilename, waveHdr);  // Save recorded audio as WAV

    free(buffer);  // Free allocated memory
    printf("Recording complete! File saved in '%s' folder.\n", RECORDINGS_FOLDER);
    return 0;
}
