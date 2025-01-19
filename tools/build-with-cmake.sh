#!/bin/bash

BUILD_TYPE=Release
CMAKE_PRESET=linux-base

help() {
    echo "build-with-cmake.sh - build the quantlib library using the cmake build system
                   [-bt|--build-type] - the type of build (Debug/Release)
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
            POSITIONAL_ARGS+=("-D$1")
            shift
    esac
done

set -- "${POSITIONAL_ARGS[@]}"

# Call basedir.sh
if [[ -z $BASEDIR ]]; then
    source ${BASH_SOURCE%/*}/basedir.sh
fi

# Validate build type
if [[ "$BUILD_TYPE" != "Release" && "$BUILD_TYPE" != "Debug" ]]; then
    echo "Invalid build type. Choose 'Release' or 'Debug'."
fi

BUILD_DIR=$BASEDIR/build/$CMAKE_PRESET

if [ -d $BUILD_DIR ]
then
    rm -rd $BUILD_DIR
fi

mkdir -p $BUILD_DIR

pushd $BUILD_DIR

echo "Running CMake with build type: $BUILD_TYPE"
echo "CMake preset: $CMAKE_PRESET"
echo "Positional arguments: ${POSITIONAL_ARGS[@]}"

cmake $BASEDIR -G "Unix Makefiles" --preset $CMAKE_PRESET -DCMAKE_BUILD_TYPE=$BUILD_TYPE "${POSITIONAL_ARGS[@]}"

CORES=$((`nproc`))

make -j$CORES

popd

# Echo results of make
if [[ $? -eq 0 ]]; then
    echo "Build succeeded."
else
    echo "Build failed."
fi
