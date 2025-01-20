#ifndef UTILITIES_H
#define UTILITIES_H

#include <sndfile.h>
#include <vector>
#include <string>

namespace Utilities {
    void getDuration(double duration);
    void displayAudioDetails(SNDFILE* sndFile);
    std::vector<float> downmixToStereo(const std::vector<float>& samples, int inputChannels, int outputChannels);
}

#endif