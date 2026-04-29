cmake_minimum_required(VERSION 3.22.1)
project(data_channel)
set(CMAKE_CXX_STANDARD 20)


set(COMMON_SRC
        src/common/semaphore.cpp
        src/common/zfp_codec.cpp
)

set(COMMON_HDR
        src/common/channel_data.h
        src/common/shared_memory_base.h
        src/common/semaphore.h
        src/common/shared_memory_producer.h
        src/common/shared_memory_consumer.h
        src/common/zfp_codec.h
        src/common/Logger.h
)

# Включаем FetchContent
include(FetchContent)

# Объявляем, что мы хотим использовать zfp
FetchContent_Declare(
        zfp
        GIT_REPOSITORY https://github.com/LLNL/zfp.git
        GIT_TAG master # или можно указать конкретную версию, например release1.0.1
)

# Настраиваем сборку zfp как статической библиотеки
# Это самые важные строки для вашего задания
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(ZFP_WITH_OPENMP OFF CACHE BOOL "" FORCE) # Отключаем OpenMP, чтобы упростить сборку
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)  # Не собираем примеры, они не нужны
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)   # Не собираем тесты

# Скачиваем и собираем библиотеку
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

