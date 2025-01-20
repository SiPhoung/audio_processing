#include <iostream>
#include <filesystem>
#include "audio_player.h"
#include "file_handler.h"

namespace fs = std::filesystem;

int main()
{
    try
    {
        std::string folderPath = std::string(Audio_DIR);

        if (!file_handler::directoryExists(folderPath))
        {
            std::cerr << "フォルダーが見つかりません: " << folderPath << std::endl;
            return EXIT_FAILURE;
        }

        size_t totalFiles = file_handler::countFilesInDirectory(folderPath);
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
                std::cout << "再生開始： " << currentFile << "\/" << totalFiles << std::endl;
                std::string filePath = entry.path().string();

                audio_player::playAudioFile(filePath);

                std::cout << std::endl << "===============" << std::endl << std::endl;
            }
        }

        std::cout << "\nすべてのファイルの再生が完了しました。" << std::endl;
        return EXIT_SUCCESS;
    }
    catch (const std::exception&)
    {
        std::cerr << "\n予期しないエラーが発生しました。" << std::endl;
        return EXIT_FAILURE;
    }
}