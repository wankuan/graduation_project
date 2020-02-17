cmake_minimum_required(VERSION 3.10)
project(demo)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_C_COMPILER gcc)
# set(CMAKE_OBJCOPY arm-none-eabi-objcopy)
# set(CMAKE_SIZE arm-none-eabi-size)
set(CMAKE_C_FLAGS "-Wall -fdata-sections -ffunction-sections -g")
# set(CMAKE_C_FLAGS_DEBUG "-Og")
# set(CMAKE_C_FLAGS_RELEASE "-O3")

SET(SRCS
    main.c
    shared_memory.c
)

SET(VERSION_INTERFACE ${VERSION})
execute_process(
	COMMAND	git log -1 --pretty=format:%H  #commit ID
	WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
	OUTPUT_VARIABLE GIT_COMMIT_ID
)
execute_process(
	COMMAND	git log -1 --pretty=format:%ci  #commit data
	WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
	OUTPUT_VARIABLE GIT_COMMIT_DATE
)
execute_process(
	COMMAND	git log -1 --pretty=format:%ae #commit email
	WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
	OUTPUT_VARIABLE GIT_AUTHOR_EMAIL
)
execute_process(
	COMMAND	git log -1 --pretty=format:%s #commit messgae
	WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
	OUTPUT_VARIABLE GIT_COMMIT_MESSAGE
)
configure_file( config.h.in
${PROJECT_SOURCE_DIR}/config.h
)
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/build/target)

add_executable(${PROJECT_NAME}.out ${SRCS})