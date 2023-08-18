add_library(utilities include/Utilities.hpp src/Utilities.cpp include/types.hpp)

if(BUILD_CATCH2)
    add_executable(test_utilities test/Utilities.test.cpp)
    target_link_libraries(test_utilities PRIVATE Catch2::Catch2WithMain utilities)
    catch_discover_tests(test_utilities)
endif()