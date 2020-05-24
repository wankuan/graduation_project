#!/bin/bash
ANDROID_NDK_PATH=/usr/android_SDK/ndk-bundle
ANDROID_NDK_LIB_PATH=/usr/android_SDK/ndk-bundle/platforms/android-20/arch-arm64/usr

cur_dir=$(cd `dirname $0`; pwd)
echo ${cur_dir}
app_running_path=${cur_dir}/app_service/test_case/app_running
inner_service_path=${cur_dir}/backstage_service/inner_service

cd ${app_running_path}
if [ ! -d build ];then
  mkdir build
fi

cd ${inner_service_path}
if [ ! -d build ];then
  mkdir build
fi

if [ "$1" = 'all' ] ;then
    echo "=================================="
    echo "========BUILD APP_RUNNING========="
    echo "=================================="
    cd ${app_running_path}/build
    cmake ..
    #   -DCMAKE_BUILD_TYPE=RELEASE\
    #   -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK_PATH/build/cmake/android.toolchain.cmake \
    #   -DANDROID_NDK=$ANDROID_NDK_PATH \
    #   -DANDROID_LIB=$ANDROID_NDK_LIB_PATH \
    #   -DANDROID_TOOLCHAIN=clang\
    #   -DANDROID_ABI=armeabi-v7a \
    #   -DANDROID_PLATFORM=android-19 \
    #   -DANDROID_STL=c++_static\
    #   ..
    make
    if [ "$?" != 0 ];then
        exit -1
    fi

    echo "=================================="
    echo "========BUILD INNER_SERVICE======="
    echo "=================================="
    cd ${inner_service_path}/build
    cmake\
      -DCMAKE_BUILD_TYPE=RELEASE\
      -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK_PATH/build/cmake/android.toolchain.cmake \
      -DANDROID_NDK=$ANDROID_NDK_PATH \
      -DANDROID_LIB=$ANDROID_NDK_LIB_PATH \
      -DANDROID_TOOLCHAIN=clang\
      -DANDROID_ABI=armeabi-v7a \
      -DANDROID_PLATFORM=android-19 \
      -DANDROID_STL=c++_static\
      ..
    make

elif [ "$1" = 'clean' ];then
    cd ${app_running_path}/build
    make clean
    rm -rf ./*

    cd ${inner_service_path}/build
    make clean
    rm -rf ./*
else
    echo "error input"
    echo "----build all->./build.sh all"
    echo "----build clean->./build.sh clean"
fi
