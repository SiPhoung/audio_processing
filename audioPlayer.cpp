#include "AudioPlayer.h"
#include "Utilities.h"
#include <iostream>
#include <cstring>

int AudioPlayer::audioCallback(const void* inputBuffer, void* outputBuffer,
    unsigned long framesPerBuffer,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void* userData)
{
    auto* data = static_cast<AudioData*>(userData);
    auto* out = static_cast<float*>(outputBuffer);

    std::memset(out, 0, framesPerBuffer * data->channels * sizeof(float));

    sf_count_t framesRead = sf_readf_float(data->sndFile, data->buffer.data(), framesPerBuffer);
    if (framesRead > 0)
    {
        if (data->sfInfo.channels > 2)
        {
            std::vector<float> downmixed = Utilities::downmixToStereo(data->buffer, data->sfInfo.channels, data->channels);
            std::memcpy(out, downmixed.data(), downmixed.size() * sizeof(float));
        }
        else
        {
            std::memcpy(out, data->buffer.data(), framesRead * data->channels * sizeof(float));
        }
    }

    return framesRead > 0 ? paContinue : paComplete;
}

void AudioPlayer::playAudioFile(const std::string& filePath)
{
    SF_INFO sfInfo{};
    SNDFILE* sndFile = sf_open(filePath.c_str(), SFM_READ, &sfInfo);
    if (!sndFile)
    {
        std::cerr << "ファイルを開けませんでした。\nファイル形式がサポートされていない、ファイル拡張子が間違っている、またはファイルが破損している可能性があります。" << std::endl;
        return;
    }

    AudioData audioData;
    audioData.sndFile = sndFile;
    audioData.sfInfo = sfInfo;
    audioData.channels = 2;
    audioData.bufferSize = 1024 * sfInfo.channels;
    audioData.buffer.resize(audioData.bufferSize);

    Utilities::getDuration(static_cast<double>(sfInfo.frames) / sfInfo.samplerate);
    Utilities::displayAudioDetails(sndFile);

    PaError err = Pa_Initialize();
    if (err != paNoError)
    {
        std::cerr << "PortAudio初期化エラー: " << Pa_GetErrorText(err) << std::endl;
        sf_close(sndFile);
        return;
    }

    PaStream* stream;
    const unsigned long BUFFER_SIZE = 1024;
    err = Pa_OpenDefaultStream(&stream, 0, audioData.channels, paFloat32, sfInfo.samplerate,
        BUFFER_SIZE, audioCallback, &audioData);
    if (err != paNoError)
    {
        std::cerr << "PortAudioストリームエラー: " << Pa_GetErrorText(err) << std::endl;
        Pa_Terminate();
        sf_close(sndFile);
        return;
    }

    err = Pa_StartStream(stream);
    if (err != paNoError)
    {
        std::cerr << "PortAudioストリーム開始エラー: " << Pa_GetErrorText(err) << std::endl;
        Pa_CloseStream(stream);
        Pa_Terminate();
        sf_close(sndFile);
        return;
    }

    std::cout << "再生中..." << std::endl;
    while (Pa_IsStreamActive(stream) == 1)
    {
        Pa_Sleep(100);
    }

    Pa_StopStream(stream);
    Pa_CloseStream(stream);
    Pa_Terminate();
    sf_close(sndFile);

    std::cout << "再生終了" << std::endl;
}