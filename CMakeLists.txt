cmake_minimum_required(VERSION 3.16)
project(aig-lite LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

enable_testing()

add_executable(test_signal tests/test_signal.cpp)
target_include_directories(test_signal PRIVATE include)
add_test(NAME signal COMMAND test_signal)

add_executable(test_node tests/test_node.cpp)
target_include_directories(test_node PRIVATE include)
add_test(NAME node COMMAND test_node)

add_executable(test_network tests/test_network.cpp src/AigNetwork.cpp)
target_include_directories(test_network PRIVATE include)
add_test(NAME network COMMAND test_network)

add_executable(test_io
    tests/test_io.cpp
    src/io/TruthTableReader.cpp
    src/io/AigerWriter.cpp
    src/io/AigerReader.cpp
    src/AigNetwork.cpp)
target_include_directories(test_io PRIVATE include)
add_test(NAME io COMMAND test_io)
