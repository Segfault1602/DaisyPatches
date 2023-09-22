#!/bin/bash

START_DIR=$PWD
LIBDAISY_DIR=$PWD/externals/libDaisy
DAISYSP_DIR=$PWD/externals/DaisySP
LIBDSP_DIR=$PWD/externals/libdsp

echo "building libDaisy . . ."
cd "$LIBDAISY_DIR" ; make -s clean ; make -j -s
if [ $? -ne 0 ]
then
    echo "Failed to compile libDaisy"
    exit 1
fi
echo "done."

echo "building DaisySP . . ."
cd "$DAISYSP_DIR" ; make -s clean ; make -j -s
if [ $? -ne 0 ]
then
    echo "Failed to compile DaisySP"
    exit 1
fi

echo "building libDSP . . ."
cd "$LIBDSP_DIR"
cmake -GNinja -Bdaisy -DCMAKE_TOOLCHAIN_FILE=toolchains/daisy.cmake
cmake --build ./daisy
if [ $? -ne 0 ]
then
    echo "Failed to compile libDSP"
    exit 1
fi
echo "done."