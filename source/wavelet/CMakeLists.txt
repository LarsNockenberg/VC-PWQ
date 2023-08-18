
add_library(wavelet include/Wavelet.hpp src/Wavelet.cpp)

if(BUILD_CATCH2)
    add_executable(test_wavelet test/Wavelet.test.cpp)
    target_link_libraries(test_wavelet PRIVATE Catch2::Catch2WithMain wavelet)
    catch_discover_tests(test_wavelet)
endif()

