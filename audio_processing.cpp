// audio_processing.cpp : Defines the entry point for the application.
//
#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <portaudio.h>
#include <sndfile.h>

namespace fs = std::filesystem;

// Struct to hold audio data and playback state
struct AudioData {
    std::vector<float> samples;
    size_t currentFrame = 0;
    int channels = 2; // Default to stereo
};

// Audio callback function
static int audioCallback(const void* inputBuffer, void* outputBuffer,
    unsigned long framesPerBuffer,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void* userData)
{
    auto* data = static_cast<AudioData*>(userData);
    auto* out = static_cast<float*>(outputBuffer);

    for (unsigned long i = 0; i < framesPerBuffer; ++i)
    {
        for (int channel = 0; channel < data->channels; ++channel)
        {
            if (data->currentFrame < data->samples.size())
            {
                *out++ = data->samples[data->currentFrame++];
            }
            else
            {
                *out++ = 0.0f; // Output silence if data ends
            }
        }
    }

    return data->currentFrame < data->samples.size() ? paContinue : paComplete;
}

// Function to play an audio file
void playAudioFile(const std::string& filePath)
{
    // Open the audio file
    SF_INFO sfInfo{};
    SNDFILE* sndFile = sf_open(filePath.c_str(), SFM_READ, &sfInfo);
    if (!sndFile)
    {
        std::cerr << "Error opening file: "
            << (sf_strerror(nullptr) ? sf_strerror(nullptr) : "Unknown error")
            << " (" << filePath << ")" << std::endl;
        return;
    }

    AudioData audioData;
    audioData.samples.resize(sfInfo.frames * sfInfo.channels);
    audioData.channels = sfInfo.channels;

    sf_readf_float(sndFile, audioData.samples.data(), sfInfo.frames);
    sf_close(sndFile);

    // Initialize PortAudio
    PaError err = Pa_Initialize();
    if (err != paNoError)
    {
        std::cerr << "PortAudio initialization error: " << Pa_GetErrorText(err) << std::endl;
        return;
    }

    // Open a PortAudio stream
    PaStream* stream;
    const unsigned long BUFFER_SIZE = 1024;
    err = Pa_OpenDefaultStream(&stream, 0, sfInfo.channels, paFloat32, sfInfo.samplerate,
        BUFFER_SIZE, audioCallback, &audioData);
    if (err != paNoError)
    {
        std::cerr << "Error opening PortAudio stream: " << Pa_GetErrorText(err) << std::endl;
        Pa_Terminate();
        return;
    }

    // Start the PortAudio stream
    err = Pa_StartStream(stream);
    if (err != paNoError)
    {
        std::cerr << "Error starting PortAudio stream: " << Pa_GetErrorText(err) << std::endl;
        Pa_CloseStream(stream);
        Pa_Terminate();
        return;
    }

    // Wait for the stream to complete
    while (Pa_IsStreamActive(stream) == 1)
    {
        Pa_Sleep(100);
    }

    // Clean up
    Pa_StopStream(stream);
    Pa_CloseStream(stream);
    Pa_Terminate();
}

int main()
{
    // Folder containing audio files
#ifndef Audio_DIR
#error "Audio_DIR is not defined. Please define it in your build system."
#endif
    std::string folderPath = std::string(Audio_DIR);

    // Check if folder exists
    if (!fs::exists(folderPath))
    {
        std::cerr << "Folder not found: " << folderPath << std::endl;
        return EXIT_FAILURE;
    }

    // Iterate through all audio files in the folder
    for (const auto& entry : fs::directory_iterator(folderPath))
    {
        if (entry.is_regular_file())
        {
            std::string filePath = entry.path().string();
            std::string fileName = entry.path().filename().string();
            std::cout << "Playing: " << fileName << std::endl;

            playAudioFile(filePath);
        }
    }

    return EXIT_SUCCESS;
}

