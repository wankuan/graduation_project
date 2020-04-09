cmake_minimum_required(VERSION 3.10)
project(inner_service)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_C_COMPILER gcc)
# set(CMAKE_OBJCOPY arm-none-eabi-objcopy)
# set(CMAKE_SIZE arm-none-eabi-size)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -fdata-sections -ffunction-sections -g -m32 -fno-stack-protector")
# set(CMAKE_C_FLAGS_DEBUG "-Og")
# set(CMAKE_C_FLAGS_RELEASE "-O3")



SET(SRCS
    ./inner_service.c
    ./../../tank_map.c
    ./../../app_service/my_osal/my_sem.c
    ./../../app_service/tank_msgq/tank_msgq.c
    ./../../app_service/tank_memory/heap_4.c
    ./../../app_service/tank_memory/tank_mm.c
    ./../../tank_components/tank_log/tank_log_api.c
    ./../../tank_components/tank_log/tank_log.c
)

include_directories(
    ./
    ./../../app_service/my_osal
    ./../../app_service/tank_msgq
    ./../../app_service/tank_memory
    ./../../app_service/tank_app
    ./../../app_service
    ./../../tank_components/tank_log/
    ./../../
    ./../
)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR})

add_executable(${PROJECT_NAME}.out ${SRCS})
target_link_libraries(${PROJECT_NAME}.out pthread rt)