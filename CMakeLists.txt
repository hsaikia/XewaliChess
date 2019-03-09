cmake_minimum_required (VERSION 2.8.11)
project (XewaliChess)

include_directories(src)
include_directories(src/Chess)
include_directories(src/NeuralNetwork)

file(GLOB SOURCES "src/*.cpp" "src/Chess/*.cpp" "src/NeuralNetwork/*.cpp")
file(GLOB HEADERS "src/*.h" "src/Chess/*.h" "src/NeuralNetwork/*.h")

add_executable(XewaliEngine ${SOURCES} ${HEADERS})

