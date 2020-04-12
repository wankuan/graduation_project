cmake_minimum_required(VERSION 3.10)
project(app_test)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_C_COMPILER gcc)
# set(CMAKE_OBJCOPY arm-none-eabi-objcopy)
# set(CMAKE_SIZE arm-none-eabi-size)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -fdata-sections -ffunction-sections -g -m32 -fno-stack-protector")
# set(CMAKE_C_FLAGS_DEBUG "-Og")
# set(CMAKE_C_FLAGS_RELEASE "-O3")



SET(SRCS
    ./app_test.c
    ./../../my_osal/my_sem.c
    ./../../tank_msgq/tank_msgq.c
    ./../../tank_memory/heap_4.c
    ./../../tank_memory/tank_mm.c
    ./../../tank_app/tank_app.c
    ./../../tank_app/tcp_fsm/tcp_fsm.c
    ./../../../tank_map.c
    ./../../../tank_components/tank_log/tank_log_api.c
    ./../../../tank_components/tank_log/tank_log.c
)

include_directories(
    ./
    ./../../my_osal
    ./../../tank_msgq
    ./../../tank_memory
    ./../../tank_app
    ./../../tank_app/tcp_fsm/
    ./../../../
    ./../../
    ./../../../tank_components/tank_log/
)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR})

add_executable(${PROJECT_NAME}.out ${SRCS})
target_link_libraries(${PROJECT_NAME}.out pthread rt)