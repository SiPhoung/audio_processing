#ifndef AUDIOPLAYER_H
#define AUDIOPLAYER_H

#include <portaudio.h>
#include <sndfile.h>
#include <vector>
#include <string>

struct AudioData {
    SNDFILE* sndFile;
    SF_INFO sfInfo;
    std::vector<float> buffer;
    size_t bufferSize;
    size_t currentFrame = 0;
    int channels = 2; // 出力チャネル数 (stereoにダウンミックス)
};

namespace audio_player {
    int audioCallback(const void* inputBuffer, void* outputBuffer,
        unsigned long framesPerBuffer,
        const PaStreamCallbackTimeInfo* timeInfo,
        PaStreamCallbackFlags statusFlags,
        void* userData);

    void playAudioFile(const std::string& filePath);
}

#endif
