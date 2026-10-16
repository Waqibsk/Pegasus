include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.7.1
)
FetchContent_MakeAvailable(Catch2)
list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)

add_executable(test_common
    test_value.cpp
    test_chunk.cpp
)
target_link_libraries(test_common PRIVATE pegasus_core Catch2::Catch2WithMain)

include(CTest)
include(Catch)
catch_discover_tests(test_common)
