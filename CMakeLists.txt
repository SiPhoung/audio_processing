# CMakeList.txt : audio_processing の CMake プロジェクト。ソースを含め、  
# プロジェクト固有のロジックをここに定義する。
#
cmake_minimum_required (VERSION 3.8)

# MSVC コンパイラがサポートしている場合、ホットリロードを有効化する。
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("audio_processing")

# このプロジェクトの実行可能ファイルにソースを追加する。
add_executable (audio_processing "audio_processing.cpp"  "audio_player.h" "audio_player.cpp" "file_handler.h" "file_handler.cpp" "utilities.h" "utilities.cpp")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET audio_processing PROPERTY CXX_STANDARD 17)
endif()

# FetchContent を含める
include(FetchContent)

# PortAudio ライブラリを取得
FetchContent_Declare(
    portaudio
    GIT_REPOSITORY https://github.com/PortAudio/portaudio.git
    GIT_TAG        master # 必要に応じて特定のタグまたはブランチに置き換え
)

# libsndfile ライブラリを取得
FetchContent_Declare(
    libsndfile
    GIT_REPOSITORY https://github.com/libsndfile/libsndfile.git
    GIT_TAG        master # 必要に応じて特定のタグまたはブランチに置き換え
)

# ライブラリを利用可能にする
FetchContent_MakeAvailable(portaudio)
FetchContent_MakeAvailable(libsndfile)

# PortAudio と libsndfile ライブラリをリンク
target_link_libraries(audio_processing PRIVATE portaudio PRIVATE sndfile)

# ライブラリのインクルードディレクトリを追加
target_include_directories(audio_processing PRIVATE
    ${portaudio_SOURCE_DIR}/include
    ${libsndfile_SOURCE_DIR}/include
)

# 音楽ディレクトリへのパスを定義
add_definitions(-DAudio_DIR="${CMAKE_SOURCE_DIR}/audio")
# TODO: 必要に応じてテストとインストールターゲットを追加
