#!/bin/bash
if [ ! -d build ];then
  mkdir build
fi
cd ./build
if [ "$1" = 'all' ] ;then
    cmake ..
    make -j64
elif [ "$1" = 'clean' ];then
    make clean
    rm -rf ./*
else
    echo "error input"
    echo "----build all->./build.sh all"
    echo "----build clean->./build.sh clean"
fi