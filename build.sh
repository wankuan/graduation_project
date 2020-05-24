#!/bin/bash
ANDROID_NDK_PATH=/home/wankuan/rk3399-firefly-industry-71-20190926/ndk
ANDROID_NDK_LIB_PATH=/usr/android_SDK/ndk-bundle/platforms/android-20/arch-arm64/usr
cd ./backstage_service/inner_service/build
if [ "$1" = 'all' ] ;then
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
    make clean
    rm -rf ./*
else
    echo "error input"
    echo "----build all->./build.sh all"
    echo "----build clean->./build.sh clean"
fi