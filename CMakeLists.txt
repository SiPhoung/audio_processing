# CMakeList.txt : CMake project for audio_processing, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("audio_processing")

# Add source to this project's executable.
add_executable (audio_processing "audio_processing.cpp" )

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET audio_processing PROPERTY CXX_STANDARD 17)
endif()

# Include FetchContent
include(FetchContent)

# Fetch PortAudio library
FetchContent_Declare(
    portaudio
    GIT_REPOSITORY https://github.com/PortAudio/portaudio.git
    GIT_TAG        master # Replace with a specific tag or branch if needed
)

# Fetch libsndfile library
FetchContent_Declare(
    libsndfile
    GIT_REPOSITORY https://github.com/libsndfile/libsndfile.git
    GIT_TAG        master # Replace with a specific tag or branch if needed
)

# Make libraries available
FetchContent_MakeAvailable(portaudio)
FetchContent_MakeAvailable(libsndfile)

# Link PortAudio and libsndfile libraries
target_link_libraries(audio_processing PRIVATE portaudio PRIVATE sndfile)

# Add include directories for libraries
target_include_directories(audio_processing PRIVATE
    ${portaudio_SOURCE_DIR}/include
    ${libsndfile_SOURCE_DIR}/include
)

# Define the path to the music directory
add_definitions(-DAudio_DIR="${CMAKE_SOURCE_DIR}/audio")
# TODO: Add tests and install targets if needed.
