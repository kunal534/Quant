cmake_minimum_required(VERSION 3.28)

project(orderbook_v2_benchmark LANGUAGES CXX)

include(FetchContent)

FetchContent_Declare(
    voided_hw_detection
    GIT_REPOSITORY https://github.com/nihilai-collective/voided-hw-detection.git
    GIT_TAG 7759798e360532e979e81dcf099979b72cf95741
)
FetchContent_Declare(
    benchmarksuite
    GIT_REPOSITORY https://github.com/nihilai-collective/benchmarksuite.git
    GIT_TAG ed43ce8c756983e9f51dac25f7fb6e24892257ec
)
FetchContent_MakeAvailable(benchmarksuite)

find_package(Threads REQUIRED)

add_executable(orderbook_v2_benchmark
    Orderbook.cpp
    orderbook_benchmark.cpp
)
target_compile_features(orderbook_v2_benchmark PRIVATE cxx_std_20)
target_link_libraries(orderbook_v2_benchmark
    PRIVATE benchmarksuite::benchmarksuite Threads::Threads)
