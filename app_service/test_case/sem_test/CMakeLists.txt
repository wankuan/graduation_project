cmake_minimum_required(VERSION 3.10)
project(sem_test)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_C_COMPILER gcc)
# set(CMAKE_OBJCOPY arm-none-eabi-objcopy)
# set(CMAKE_SIZE arm-none-eabi-size)
set(CMAKE_C_FLAGS "-Wall -fdata-sections -ffunction-sections -g")
# set(CMAKE_C_FLAGS_DEBUG "-Og")
# set(CMAKE_C_FLAGS_RELEASE "-O3")

set(CMAKE_C_FLAGS "-pthread")

SET(SRCS
    sem_test.c
    ./../../my_osal/my_sem.c
)

include_directories(
    ./
    ./../../my_osal
    ./../..
)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR})

add_executable(${PROJECT_NAME}.out ${SRCS})