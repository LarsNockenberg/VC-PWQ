cmake_minimum_required(VERSION 3.4)
set (CMAKE_CXX_STANDARD 17)

project(VC_PWQ)

option(CLANG_TIDY "Enable Clang Tidy checks" OFF)
option(BUILD_PYBIND11 "Enable Pybind11" OFF)

include(cmake/catch2.cmake)

find_package(PkgConfig REQUIRED)
pkg_search_module(FFTW REQUIRED fftw3 IMPORTED_TARGET)

include(FetchContent)

FetchContent_Declare(
  AudioFile
  GIT_REPOSITORY https://github.com/adamstark/AudioFile.git
  GIT_TAG        1.1.0
)

include_directories(AudioFile)
FetchContent_MakeAvailable(AudioFile)
set_target_properties(Tests PROPERTIES EXCLUDE_FROM_ALL TRUE)


if (BUILD_CATCH2)
    include(CTest)
    include(Catch)
    enable_testing()
endif()

if(CLANG_TIDY)
    set(CMAKE_CXX_CLANG_TIDY "clang-tidy")
endif()

include_directories(source)
add_subdirectory(source)