FROM alpine AS builder

RUN apk update && apk add g++ make cmake python3 python3-dev py3-pip ffmpeg-dev && pip install pybind11[global]

WORKDIR /work
COPY src/ src/
COPY include/ include/
COPY CMakeLists.txt ./

RUN mkdir build && cd build && cmake .. && make -j$(nproc) && make install