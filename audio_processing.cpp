#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <portaudio.h>
#include <sndfile.h>

namespace fs = std::filesystem;

// AudioData構造体: オーディオデータと再生状態を保持
struct AudioData {
    std::vector<float> samples;
    size_t currentFrame = 0;
    int channels = 2;// チャネル数 (デフォルトはstereo)
};

// (PaStreamCallback型)オーディオコールバック関数: サンプルデータをバッファにコピー
static int audioCallback(const void* inputBuffer, void* outputBuffer,
    unsigned long framesPerBuffer,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void* userData)
{
    auto* data = static_cast<AudioData*>(userData); // ユーザーデータをAudioDataにキャスト
    auto* out = static_cast<float*>(outputBuffer); // 出力バッファ

    for (unsigned long i = 0; i < framesPerBuffer; ++i)
    {
        for (int channel = 0; channel < data->channels; ++channel)
        {
            if (data->currentFrame < data->samples.size())
            {
                *out++ = data->samples[data->currentFrame++]; // サンプルをコピー
            }
            else
            {
                *out++ = 0.0f; // データが終了したら無音を出力
            }
        }
    }

    // 再生が終了したかを判断
    return data->currentFrame < data->samples.size() ? paContinue : paComplete;
}

// オーディオファイルを再生する関数
void playAudioFile(const std::string& filePath)
{
    // オーディオファイルを開く
    SF_INFO sfInfo{};
    SNDFILE* sndFile = sf_open(filePath.c_str(), SFM_READ, &sfInfo);
    if (!sndFile)
    {
        std::cerr << "ファイルを開けませんでした: "
            << (sf_strerror(nullptr) ? sf_strerror(nullptr) : "不明なエラー")
            << " (" << filePath << ")" << std::endl;
        return;
    }

    // AudioDataを初期化
    AudioData audioData;
    audioData.samples.resize(sfInfo.frames * sfInfo.channels);
    audioData.channels = sfInfo.channels;

    // ファイルデータを読み込む
    sf_readf_float(sndFile, audioData.samples.data(), sfInfo.frames);
    sf_close(sndFile);

    // PortAudioを初期化
    PaError err = Pa_Initialize();
    if (err != paNoError)
    {
        std::cerr << "PortAudio初期化エラー: " << Pa_GetErrorText(err) << std::endl;
        return;
    }

    // PortAudioストリームを開く
    PaStream* stream;
    const unsigned long BUFFER_SIZE = 1024; // バッファサイズ
    err = Pa_OpenDefaultStream(&stream, 0, sfInfo.channels, paFloat32, sfInfo.samplerate,
        BUFFER_SIZE, audioCallback, &audioData);
    if (err != paNoError)
    {
        std::cerr << "PortAudioストリームエラー: " << Pa_GetErrorText(err) << std::endl;
        Pa_Terminate();
        return;
    }

    // PortAudioストリームを開始
    err = Pa_StartStream(stream);
    if (err != paNoError)
    {
        std::cerr << "PortAudioストリーム開始エラー: " << Pa_GetErrorText(err) << std::endl;
        Pa_CloseStream(stream);
        Pa_Terminate();
        return;
    }

    std::cout << "再生中..." << std::endl;

    // 再生が終了するまで待つ
    while (Pa_IsStreamActive(stream) == 1)
    {
        Pa_Sleep(100); // 100ms待機
    }

    // クリーンアップ
    Pa_StopStream(stream);
    Pa_CloseStream(stream);
    Pa_Terminate();

    std::cout << "再生終了: " << filePath << std::endl;
}

int main()
{
    // オーディオファイルが保存されているフォルダー
#ifndef Audio_DIR
#error "Audio_DIRが定義されていません。ビルドシステムで定義してください。"
#endif
    std::string folderPath = std::string(Audio_DIR);

    // フォルダーの存在を確認
    if (!fs::exists(folderPath))
    {
        std::cerr << "フォルダーが見つかりません: " << folderPath << std::endl;
        return EXIT_FAILURE;
    }

    // フォルダー内のすべてのオーディオファイルを再生
    for (const auto& entry : fs::directory_iterator(folderPath))
    {
        if (entry.is_regular_file())
        {
            std::string filePath = entry.path().string();
            std::string fileName = entry.path().filename().string();
            std::cout << "再生開始: " << fileName << std::endl;

            playAudioFile(filePath);
        }
    }

    std::cout << "すべてのファイルの再生が完了しました。" << std::endl;

    return EXIT_SUCCESS;
}
