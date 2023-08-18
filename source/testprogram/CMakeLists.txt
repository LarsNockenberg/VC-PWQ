add_executable(VC_PWQ src/main.cpp)
target_link_libraries(VC_PWQ encoder decoder PkgConfig::FFTW)
