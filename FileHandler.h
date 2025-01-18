#ifndef FILEHANDLER_H
#define FILEHANDLER_H

#include <filesystem>
#include <string>

namespace FileHandler {
    bool directoryExists(const std::string& path);
    size_t countFilesInDirectory(const std::string& path);
}

#endif