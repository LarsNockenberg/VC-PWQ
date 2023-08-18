
add_library(decoder include/Decoder.hpp src/Decoder.cpp include/DecoderInterface.hpp src/DecoderInterface.cpp)
target_link_libraries(decoder psychohapticModel wavelet utilities losslessCoding AudioFile)
