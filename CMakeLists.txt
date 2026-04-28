cmake_minimum_required(VERSION 4.0)
project(data_channel)

set(CMAKE_CXX_STANDARD 20)

add_executable(data_channel producer.cpp
        i_compressor.h
        channel_data.h
        mantissa_compressor.cpp)
