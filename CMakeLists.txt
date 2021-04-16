cmake_minimum_required (VERSION 2.8.11)
project (XewaliChess)

include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(${PROJECT_SOURCE_DIR}/src/Chess)

file(GLOB SOURCESSF ${PROJECT_SOURCE_DIR}/src/Chess/*.cpp)
file(GLOB HEADERSSF ${PROJECT_SOURCE_DIR}/src/Chess/*.h)
file(GLOB SOURCESX ${PROJECT_SOURCE_DIR}/src/Xewali/*.cpp)
file(GLOB HEADERSX ${PROJECT_SOURCE_DIR}/src/Xewali/*.h)

add_executable(XewaliEngine ${SOURCESSF} ${SOURCESX} ${HEADERSSF} ${HEADERSX} ${PROJECT_SOURCE_DIR}/src/main.cpp)

