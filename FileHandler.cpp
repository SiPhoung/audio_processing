#include "FileHandler.h"
#include <filesystem>

namespace fs = std::filesystem;

bool FileHandler::directoryExists(const std::string& path)
{
    return fs::exists(path);
}

size_t FileHandler::countFilesInDirectory(const std::string& path)
{
    size_t count = 0;
    for (const auto& entry : fs::directory_iterator(path))
    {
        if (entry.is_regular_file())
            count++;
    }
    return count;
}