


# cmake_minimum_required(VERSION 3.10)
# project(external_service)
# set_property(GLOBAL PROPERTY USE_FOLDERS ON)
# set(CMAKE_C_COMPILER gcc)
# # set(CMAKE_OBJCOPY arm-none-eabi-objcopy)
# # set(CMAKE_SIZE arm-none-eabi-size)
# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -fdata-sections -ffunction-sections -g -m32 -fno-stack-protector")
# # set(CMAKE_C_FLAGS_DEBUG "-Og")
# # set(CMAKE_C_FLAGS_RELEASE "-O3")



# SET(SRCS
#     ./external_process_service.c
#     ./external_process.c
#     ./../../tank_map.c
#     ./../../app_service/my_osal/my_sem.c
#     ./../../app_service/my_list/my_list.c
#     ./../../app_service/tank_msgq/tank_msgq.c
#     ./../../app_service/tank_memory/heap_4.c
#     ./../../app_service/tank_memory/tank_mm.c
#     ./../../tank_components/tank_log/tank_log_api.c
#     ./../../tank_components/tank_log/tank_log.c
#     ./../../tank_components/delay/tank_delay.c
# )

# include_directories(
#     ./
#     ./../../app_service/my_osal
#     ./../../app_service/my_list
#     ./../../app_service/tank_msgq
#     ./../../app_service/tank_memory
#     ./../../app_service/tank_app
#     ./../../app_service
#     ./../../
#     ./../
#     ./../../tank_components/tank_log
#     ./../../tank_components/delay
# )

# SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR})

# add_executable(${PROJECT_NAME}.out ${SRCS})
# target_link_libraries(${PROJECT_NAME}.out pthread rt)

# CMAKE_MINIMUM_REQUIRED(VERSION 2.9)
# PROJECT(demo)
# SET(BUILD_NAME ${PROJECT_NAME})

IF (${CMAKE_HOST_SYSTEM_NAME} MATCHES "Windows")
    ADD_DEFINITIONS(-DWindows)
ELSE (${CMAKE_HOST_SYSTEM_NAME} MATCHES "Linux")
    ADD_DEFINITIONS(-DLinux)
ENDIF ()


set(ANDROID_SDK_INC
/home/wankuan/rk3399-firefly-industry-71-20190926/system/core/include
/usr/android_SDK/ndk-bundle/sysroot/usr/include
/usr/android_SDK/ndk-bundle/sysroot/usr/lib/aarch64-linux-android
)
message(${ANDROID_SDK_INC})
ADD_DEFINITIONS(-D__ANDROID__)

IF (${CMAKE_BUILD_TYPE} MATCHES "DEBUG")
    message( STATUS "==============================debug==============================" )
    ADD_DEFINITIONS(-DDEBUG)
ELSE(${CMAKE_BUILD_TYPE} MATCHES "RELEASE")
    message( STATUS "==============================release==============================" )
ENDIF ()


#SET(CMAKE_CXX_FLAGS "-std=gnu++11 -Wall -fPIC -Wl,-Bsymbolic -lm -D__STDINT_LIMITS -D__STDINT_MACROS -D__ANDROID__ -DANDROID")

cmake_minimum_required(VERSION 3.10)
project(inner_service)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
# set(CMAKE_C_COMPILER gcc)
# set(CMAKE_OBJCOPY arm-none-eabi-objcopy)
# set(CMAKE_SIZE arm-none-eabi-size)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11 -Wall -fdata-sections -ffunction-sections -g -m32 -fno-stack-protector")
# set(CMAKE_C_FLAGS_DEBUG "-Og")
# set(CMAKE_C_FLAGS_RELEASE "-O3")



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

link_directories(./../../lib)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR})


# add_library(${PROJECT_NAME}  ${SRCS})
add_executable(${PROJECT_NAME}.out ${SRCS})
target_link_libraries(${PROJECT_NAME}.out  libcutils.so  ${pthread})
© 2020 GitHub, Inc.