cmake_minimum_required(VERSION 3.10)
project(OrderBookTest)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Google Test
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

# Add your orderbook source
add_library(orderbook orderbook.cpp)

# Add the test executable
add_executable(orderbook_test orderbook_test_basic.cpp)

# Link libraries
target_link_libraries(orderbook_test 
    orderbook
    ${GTEST_LIBRARIES}
    pthread
)

# Enable testing
enable_testing()
add_test(NAME orderbook_test COMMAND orderbook_test)