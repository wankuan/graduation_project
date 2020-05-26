cmake_minimum_required(VERSION 3.10)
project(inner_service)

message("=====${RUN_PLATFORM}")

IF (${RUN_PLATFORM} STREQUAL "ANDROID")
    message("======BUILD ANDROID======")
    set(ANDROID_SDK_INC
    /usr/android_SDK/ndk-bundle/sysroot/usr/include
    /usr/android_SDK/ndk-bundle/sysroot/usr/lib/aarch64-linux-android
    )
    message(${ANDROID_SDK_INC})
    ADD_DEFINITIONS(-D__ANDROID__)
ELSEIF(${RUN_PLATFORM} STREQUAL "LINUX")
    message("======BUILD LINUX======")
    set(CMAKE_C_COMPILER gcc)
ENDIF()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11 -Wall -fdata-sections -ffunction-sections -g -m32 -fno-stack-protector")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -fdata-sections -ffunction-sections -g -m32 -fno-stack-protector")


SET(SRCS
    ./inner_service.c
    ./../../tank_map.c
    ./../../app_service/my_osal/my_sem.c
    ./../../app_service/my_list/my_list.c
    ./../../app_service/tank_msgq/tank_msgq.c
    ./../../app_service/tank_memory/heap_4.c
    ./../../app_service/tank_memory/tank_mm.c
    ./../../tank_components/tank_log/tank_log_api.c
    ./../../tank_components/tank_log/tank_log.c
    ./../../tank_components/delay/tank_delay.c
)

include_directories(
    ./
    ./../../app_service/my_osal
    ./../../app_service/my_list
    ./../../app_service/tank_msgq
    ./../../app_service/tank_memory
    ./../../app_service/tank_app
    ./../../app_service
    ./../../
    ./../
    ./../../tank_components/tank_log
    ./../../tank_components/delay
    ${ANDROID_SDK_INC}
    ${ANDROID_LIB}
)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR})


add_executable(${PROJECT_NAME}.out ${SRCS})


IF (${RUN_PLATFORM} STREQUAL "ANDROID")
    target_link_libraries(${PROJECT_NAME}.out ${pthread})
ELSEIF(${RUN_PLATFORM} STREQUAL "LINUX")
    target_link_libraries(${PROJECT_NAME}.out pthread rt)
ENDIF()
