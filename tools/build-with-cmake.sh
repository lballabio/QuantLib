#!/bin/bash

BUILD_TYPE=Release
CMAKE_PRESET=linux-base

help() {
    echo "build-with-cmake.sh [-bt|--build-type] - the type of build (Debug/Release)
                   [-p|--preset] - cmake-preset to use
                   [-h|--help] - Output the help message 
    "
    return
}

POSITIONAL_ARGS=()

while [[ "$#" -gt 0 ]]; do
    case $1 in
        -bt|--build-type)
            BUILD_TYPE="$2"
            shift 2
            ;;
        -p|--preset)
            CMAKE_PRESET="$2"
            shift 2
            ;;
        -h|--help)
            help
            exit 0
            ;;
        -*|--*)
            echo "Unknown parameter passed: $1"
            exit 1
            ;;
        *)
            POSITIONAL_ARGS+=("$1")
            shift
    esac
done

# Call basedir.sh
if [[ -z $BASEDIR ]]; then
    source ${BASH_SOURCE%/*}/basedir.sh
fi

# Validate build type
if [[ "$BUILD_TYPE" != "Release" && "$BUILD_TYPE" != "Debug" ]]; then
    echo "Invalid build type. Choose 'Release' or 'Debug'."
fi

BUILD_DIR=$BASEDIR/build/

if [ -d $BUILD_DIR ]
then
    rm -rd $BUILD_DIR
fi

mkdir -p $BUILD_DIR

pushd $BUILD_DIR

echo "Running CMake with build type: $BUILD_TYPE"
echo "Using compiler: $COMPILER"
echo "$COMPILER Version: $GCC_VERSION"
cmake $BASEDIR -G "Unix Makefiles" --preset $CMAKE_PRESET -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DUSE_CLANG=$USE_CLANG

CORES=$((`nproc`))

make -j$CORES

popd

# Echo results of make
if [[ $? -eq 0 ]]; then
    echo "Build succeeded."
else
    echo "Build failed."
fi
