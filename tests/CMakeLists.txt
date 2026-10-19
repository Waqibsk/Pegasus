include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.7.1
)
FetchContent_MakeAvailable(Catch2)
list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)

set(TEST_SOURCES
    test_value.cpp
    test_chunk.cpp
)

include(CTest)
include(Catch)

# One executable per test file, so a single file can be built and run:
#   make test-file FILE=test_value
foreach(test_src IN LISTS TEST_SOURCES)
    get_filename_component(test_name ${test_src} NAME_WE)
    add_executable(${test_name} ${test_src})
    target_link_libraries(${test_name} PRIVATE pegasus_core Catch2::Catch2WithMain)
    catch_discover_tests(${test_name})
endforeach()

# Combined binary with every test: ./build/tests/test_common
add_executable(test_common ${TEST_SOURCES})
target_link_libraries(test_common PRIVATE pegasus_core Catch2::Catch2WithMain)
