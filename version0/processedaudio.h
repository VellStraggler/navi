#ifndef PROCESSEDAUDIO_H
#define PROCESSEDAUDIO_H

#include <iostream>
#include <cmath>
#include "miniaudio.h"


class ProcessedAudio {
private:
    const char* audioFileName;
    float latestVolume;

    ma_decoder decoder;
    ma_device device;

    void handle_callback(float* samples, ma_uint32 frameCount, ma_uint32 channels);
    static void data_callback_proxy(ma_device* device, void* output, const void* input, ma_uint32 frameCount);

public:
    ProcessedAudio(const char* fileName);
    int run();

    // Call this only after playback stopped or from callback thread to safely read volume
    float getLatestVolume() const;
};

#endif // PROCESSEDAUDIO_H
