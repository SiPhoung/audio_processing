#include "Utilities.h"
#include <iostream>
#include <iomanip>
#include <cstring>

void utilities::getDuration(double duration)
{
    int hours = static_cast<int>(duration) / 3600;
    int minutes = (static_cast<int>(duration) % 3600) / 60;
    int seconds = static_cast<int>(duration) % 60;

    std::cout << "�Đ����ԁF "
        << std::setw(2) << std::setfill('0') << hours << ":"
        << std::setw(2) << std::setfill('0') << minutes << ":"
        << std::setw(2) << std::setfill('0') << seconds << std::endl;
}

void utilities::displayAudioDetails(SNDFILE* sndFile)
{
    const int charSize = 256;
    char artist[charSize];
    char title[charSize];
    char album[charSize];

    if (sf_get_string(sndFile, SF_STR_TITLE))
    {
        strncpy(title, sf_get_string(sndFile, SF_STR_TITLE), charSize);
        std::cout << "  �Ȗ�: " << title << std::endl;
    }
    else
    {
        std::cout << "  �Ȗ�: �s��" << std::endl;
    }

    if (sf_get_string(sndFile, SF_STR_ARTIST))
    {
        strncpy(artist, sf_get_string(sndFile, SF_STR_ARTIST), charSize);
        std::cout << "  �A�[�e�B�X�g�F�@" << artist << std::endl;
    }
    else
    {
        std::cout << "  �A�[�e�B�X�g�F�@�s��" << std::endl;
    }

    if (sf_get_string(sndFile, SF_STR_ALBUM))
    {
        strncpy(album, sf_get_string(sndFile, SF_STR_ALBUM), charSize);
        std::cout << "  �A���o���F�@" << album << std::endl;
    }
    else
    {
        std::cout << "  �A���o���F�@�s��" << std::endl;
    }
}

// ���ׂĂ̓��̓`���l�������ƉE�̃`���l���ɋϓ��Ƀ~�b�N�X
std::vector<float> utilities::downmixToStereo(const std::vector<float>& samples, int inputChannels, int outputChannels)
{
    size_t totalFrames = samples.size() / inputChannels;
    std::vector<float> downmixedSamples(totalFrames * outputChannels, 0.0f);

    for (size_t frame = 0; frame < totalFrames; ++frame)
    {
        for (int inChannel = 0; inChannel < inputChannels; ++inChannel)
        {
            downmixedSamples[frame * outputChannels + 0] += samples[frame * inputChannels + inChannel] * (1.0f / inputChannels); //Left Channel
            downmixedSamples[frame * outputChannels + 1] += samples[frame * inputChannels + inChannel] * (1.0f / inputChannels); //Right Channel
        }
    }

    return downmixedSamples;
}