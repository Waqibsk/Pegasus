include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.7.1
)
FetchContent_MakeAvailable(Catch2)
list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)

set(TEST_SOURCES
    ./common/test_value.cpp
    ./common/test_chunk.cpp
    ./execution/test_filter.cpp
    ./execution/test_projection.cpp
    ./execution/test_table_scan.cpp
    ./execution/test_aggregate.cpp
    ./catalog/test_catalog.cpp
)

include(CTest)
include(Catch)

# One executable per test file, so a single file can be built and run:
#   make test-file FILE=test_value
foreach(test_src IN LISTS TEST_SOURCES)
    get_filename_component(test_name ${test_src} NAME_WE)
    add_executable(${test_name} ${test_src})
    target_link_libraries(
        ${test_name}
        PRIVATE pegasus_core Catch2::Catch2WithMain
    )
    catch_discover_tests(${test_name})
endforeach()

# Combined binary with every test: ./build/tests/test_all
add_executable(test_all ${TEST_SOURCES})
target_link_libraries(test_all PRIVATE pegasus_core Catch2::Catch2WithMain)
catch_discover_tests(test_all)
