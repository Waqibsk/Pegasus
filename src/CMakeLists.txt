add_library(
    pegasus_core
    ./execution/aggregate_operator.cpp
    ./execution/filter_operator.cpp
    ./execution/project_operator.cpp
    ./execution/table_scan_operator.cpp
)

target_include_directories(pegasus_core PUBLIC ${CMAKE_SOURCE_DIR}/src)
add_executable(pegasus main.cpp)
target_link_libraries(pegasus PRIVATE pegasus_core)
