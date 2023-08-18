
add_library(encoder include/Encoder.hpp src/Encoder.cpp include/EncoderInterface.hpp src/EncoderInterface.cpp)
target_link_libraries(encoder psychohapticModel wavelet utilities losslessCoding AudioFile)
