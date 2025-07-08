#include <iostream>
#include <cmath>
#include "miniaudio.h"
#include <chrono>

class ProcessedAudio {
private:
    const char* audioFileName;
    float latestVolume = 0.0f;

    ma_decoder decoder;
    ma_device device;

    // Calculate average volume (absolute amplitude) inside callback
    void handle_callback(float* samples, ma_uint32 frameCount, ma_uint32 channels) {
        float sum = 0.0f;
        for (ma_uint32 i = 0; i < frameCount * channels; ++i) {
            sum += std::fabs(samples[i]);
        }
        latestVolume = sum / (frameCount * channels);
    }

    static void data_callback_proxy(ma_device* device, void* output, const void* input, ma_uint32 frameCount) {
        auto* self = static_cast<ProcessedAudio*>(device->pUserData);
        ma_decoder* decoder = &self->decoder;

        ma_uint64 framesRead = 0;
        ma_decoder_read_pcm_frames(decoder, output, frameCount, &framesRead);
        if (framesRead < frameCount) {
            float* samples = (float*)output;
            std::fill(samples + framesRead * device->playback.channels,
                      samples + frameCount * device->playback.channels,
                      0.0f);
        }

        self->handle_callback((float*)output, frameCount, device->playback.channels);
    }

public:
    ProcessedAudio(const char* fileName) : audioFileName(fileName) {}

    int run() {
        ma_result result;

        result = ma_decoder_init_file(audioFileName, NULL, &decoder);
        if (result != MA_SUCCESS) {
            std::cerr << "Failed to load audio file.\n";
            return -1;
        }

        ma_device_config config = ma_device_config_init(ma_device_type_playback);
        config.playback.format   = decoder.outputFormat;
        config.playback.channels = decoder.outputChannels;
        config.sampleRate        = decoder.outputSampleRate;
        config.dataCallback      = data_callback_proxy;
        config.pUserData         = this;
        config.noClip            = MA_TRUE;

        result = ma_device_init(NULL, &config, &device);
        if (result != MA_SUCCESS) {
            std::cerr << "Failed to initialize playback device.\n";
            ma_decoder_uninit(&decoder);
            return -1;
        }

        result = ma_device_start(&device);
        if (result != MA_SUCCESS) {
            std::cerr << "Failed to start playback.\n";
            ma_device_uninit(&device);
            ma_decoder_uninit(&decoder);
            return -1;
        }

        std::cout << "Playing... (Ctrl+C to quit)\n";

        // Playback loop — no mutex, no thread sync
        while (true) {
            // WARNING: Reading latestVolume here is not thread-safe,
            // so ideally read only when playback stops or from the callback thread.
            std::cout << "Volume: " << latestVolume << std::endl;
           
            int64_t t1 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count() + (1000.0/60.0);
            int64_t t2 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
            while (t2 < t1) {
                t2 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
            }
            // ma_sleep(1000 / 60); // MiniAudio's cross-platform sleep
            
        }
        
        ma_device_uninit(&device);
        ma_decoder_uninit(&decoder);
        return 0;
    }
    
    // Getter for volume after playback ends (safe to call then)
    float getLatestVolume() const {
        return latestVolume;
    }
};
