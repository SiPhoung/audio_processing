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
    int channels = 2; // 出力チャネル数 (stereoにダウンミックス)
};

// マルチチャネルオーディオをステレオにダウンミックス
std::vector<float> downmixToStereo(const std::vector<float>& samples, int inputChannels, int outputChannels)
{
    size_t totalFrames = samples.size() / inputChannels;
    std::vector<float> downmixedSamples(totalFrames * outputChannels, 0.0f);

    for (size_t frame = 0; frame < totalFrames; ++frame)
    {
        for (int inChannel = 0; inChannel < inputChannels; ++inChannel)
        {
            // すべての入力チャネルを左と右のチャネルに均等にミックス
            downmixedSamples[frame * outputChannels + 0] += samples[frame * inputChannels + inChannel] * (1.0f / inputChannels); // Left channel
            downmixedSamples[frame * outputChannels + 1] += samples[frame * inputChannels + inChannel] * (1.0f / inputChannels); // Right channel
        }
    }

    return downmixedSamples;
}

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
        const char* errorMsg = sf_strerror(nullptr);
        std::cerr << "ファイルを開けませんでした。 " << std::endl;

        switch (sf_error(nullptr))
        {
        case 18:
                std::cerr << "エラー: ファイルには実装されていない形式のデータが含まれています。" << std::endl;
                break;

            case SF_ERR_UNSUPPORTED_ENCODING:
                std::cerr << "エラー: ファイルはサポートされていないエンコーディングを使用しています。" << std::endl;
                break;

            case SF_ERR_UNRECOGNISED_FORMAT:
                std::cerr << "エラー: 認識できないファイル形式です。" << std::endl;
                break;

            case SF_ERR_MALFORMED_FILE:
                std::cerr << "エラー: ファイルが破損しています。" << std::endl;
                break;

            case SF_ERR_SYSTEM:
                std::cerr << "エラー: システムエラーが発生しました。" << std::endl;
                break;

            default:
                std::cerr << "エラー: 不明なエラーが発生しました。" << std::endl;
                break;
        }

        return;
    }

    // AudioDataを初期化
    AudioData audioData;
    audioData.samples.resize(sfInfo.frames * sfInfo.channels);
    audioData.channels = 2; // 出力はステレオに固定
    std::cout << sfInfo.samplerate << std::endl;
    // ファイルデータを読み込む
    sf_readf_float(sndFile, audioData.samples.data(), sfInfo.frames);
    const int charSize = 256;
    char artist[charSize];
    char title[charSize];
    char album[charSize];

    if (sf_get_string(sndFile, SF_STR_TITLE))
    {
        strncpy(title, sf_get_string(sndFile, SF_STR_TITLE), charSize);
        std::cout << "曲名: " << title << std::endl;
    }
    else
    {
        std::cout << "曲名: 不明" << std::endl;
    }

    if (sf_get_string(sndFile, SF_STR_ARTIST))
    {
        strncpy(artist, sf_get_string(sndFile, SF_STR_ARTIST), charSize);
        std::cout << "アーティスト：　" << artist << std::endl;
    }
    else
    {
        std::cout << "アーティスト：　不明" << std::endl;
    }

    if (sf_get_string(sndFile, SF_STR_ALBUM))
    {
        strncpy(album, sf_get_string(sndFile, SF_STR_ALBUM), charSize);
        std::cout << "アルバム：　" << album << std::endl;
    }
    else
    {
        std::cout << "アルバム：　不明" << std::endl;
    }

    // マルチチャネルオーディオをステレオにダウンミックス
    if (sfInfo.channels > 2)
        audioData.samples = downmixToStereo(audioData.samples, sfInfo.channels, audioData.channels);

    // PortAudioを初期化
    PaError err = Pa_Initialize();
    if (err != paNoError)
    {
        std::cerr << "PortAudio初期化エラー: " << Pa_GetErrorText(err) << std::endl;
        return;
    }

    // PortAudioストリームを開く
    PaStream* stream;
    const unsigned long BUFFER_SIZE = 1024;
    err = Pa_OpenDefaultStream(&stream, 0, audioData.channels, paFloat32, sfInfo.samplerate,
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
        Pa_Sleep(100);
    }

    // クリーンアップ
    Pa_StopStream(stream);
    Pa_CloseStream(stream);
    Pa_Terminate();
    sf_close(sndFile);

    std::cout << "再生終了" << std::endl;
}

int main()
{
    try
    {
        std::string folderPath = std::string(Audio_DIR);

        if (!fs::exists(folderPath))
        {
            std::cerr << "フォルダーが見つかりません: " << folderPath << std::endl;
            return EXIT_FAILURE;
        }

        size_t totalFiles = 0;
        for (const auto& entry : fs::directory_iterator(folderPath)) {
            if (entry.is_regular_file())
                totalFiles++;
        }

        if (totalFiles == 0) 
        {
            std::cerr << "フォルダーで音声のファイルが存在しません。" << std::endl;
            return EXIT_FAILURE;
        }

        size_t currentFile = 0;
        for (const auto& entry : fs::directory_iterator(folderPath))
        {
            if (entry.is_regular_file())
            {
                currentFile++;
                std::cout << "再生開始 : " << currentFile << "\/" << totalFiles << std::endl;
                std::string filePath = entry.path().string();
                std::string fileName = entry.path().filename().string();

                playAudioFile(filePath);
            }
        }

        std::cout << "すべてのファイルの再生が完了しました。" << std::endl;

        return EXIT_SUCCESS;
    }
    catch (const std::exception&)
    {
        std::cerr << "予期しないエラーが発生しました。" << std::endl;
        return EXIT_FAILURE;
    }
}
