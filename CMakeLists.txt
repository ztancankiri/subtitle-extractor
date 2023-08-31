cmake_minimum_required(VERSION 3.0)

project(main)

set (CMAKE_CXX_STANDARD 14)

include_directories(include)

find_path(AVFORMAT_INCLUDE_DIR libavformat/avformat.h)
find_library(AVFORMAT_LIBRARY avformat)

find_path(AVCODEC_INCLUDE_DIR libavcodec/avcodec.h)
find_library(AVCODEC_LIBRARY avcodec)

add_executable(main src/main.cpp)
target_include_directories(main PRIVATE ${AVFORMAT_INCLUDE_DIR} ${AVCODEC_INCLUDE_DIR})
target_link_libraries(main PRIVATE ${AVFORMAT_LIBRARY} ${AVCODEC_LIBRARY})