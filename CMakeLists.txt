cmake_minimum_required(VERSION 3.10)
project(subextractor)

# Settings
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DQT_DBUS_LIB -DQT_CORE_LIB -Wnon-virtual-dtor -Wextra -Wno-missing-field-initializers -Wunused -Wpedantic -Wno-maybe-uninitialized -Wlogical-op -Wno-extra-semi -Wmisleading-indentation -Wduplicated-cond -Wshadow-compatible-local -Wduplicated-branches -Wno-deprecated-copy -Wno-stringop-overflow")
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(PYBIND11_PYTHON_VERSION "3.11")

# Project Headers & Sources
include_directories(include)
file(GLOB_RECURSE SOURCES src/*.cpp)

# avformat library
find_path(AVFORMAT_INCLUDE_DIR libavformat/avformat.h)
find_library(AVFORMAT_LIBRARY avformat)

# avcodec library
find_path(AVCODEC_INCLUDE_DIR libavcodec/avcodec.h)
find_library(AVCODEC_LIBRARY avcodec)

# Python3.11 & pybind11
find_package(Python 3.11 COMPONENTS Interpreter Development)
find_package(pybind11 REQUIRED)

# Build module
pybind11_add_module(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE ${AVFORMAT_INCLUDE_DIR} ${AVCODEC_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE 
    ${AVFORMAT_LIBRARY}
    ${AVCODEC_LIBRARY}
    Python::Python
    "-static-libstdc++"
)