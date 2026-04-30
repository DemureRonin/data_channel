cmake_minimum_required(VERSION 3.22.1)
project(data_channel)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

set(COMMON_SRC
        src/common/semaphore.cpp
        src/common/zfp_codec.cpp
)

set(COMMON_HDR
        src/common/channel_data.h
        src/common/semaphore.h
        src/common/shared_memory_producer.h
        src/common/shared_memory_consumer.h
        src/common/zfp_codec.h
)

include(FetchContent)

FetchContent_Declare(
        zfp
        GIT_REPOSITORY https://github.com/LLNL/zfp.git
        GIT_TAG master
)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(ZFP_WITH_OPENMP OFF CACHE BOOL "" FORCE)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(zfp)

add_executable(producer
        src/producer/producer.cpp
        src/producer/producer_core.cpp
        ${COMMON_SRC}
        ${COMMON_HDR}
)

add_executable(consumer
        src/consumer/consumer.cpp
        src/consumer/consumer_core.cpp
        ${COMMON_SRC}
        ${COMMON_HDR}
)

target_link_libraries(producer PRIVATE zfp::zfp pthread)
target_link_libraries(consumer PRIVATE zfp::zfp pthread)


target_link_options(producer PRIVATE -fuse-ld=lld)
target_link_options(consumer PRIVATE -fuse-ld=lld)