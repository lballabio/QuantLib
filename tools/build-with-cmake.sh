#!/bin/bash

BUILD_TYPE=Release
CMAKE_PRESET=linux-base

help() {
    echo "build-with-cmake.sh - build the QuantLib library using the CMake build system
    Usage: build-with-cmake.sh [options] [positional arguments]
    
    Options:
        -bt|--build-type <type>    Specify the build type (Debug/Release). Default: Release
        -p|--preset <preset>       Specify the CMake preset to use. Default: linux-base
        -h|--help                  Output this help message
    
    Positional Arguments:
        These are additional options passed to CMake as '-D<argument>=<value>' pairs.
        Examples:
            QL_BUILD_BENCHMARK=ON   Enable building of benchmarks
            QL_BUILD_EXAMPLES=OFF   Disable building of examples
            QL_BUILD_TEST_SUITE=ON  Enable building of the test suite
    
    Example Usage:
        ./build-with-cmake.sh -bt Release -p linux-base QL_BUILD_BENCHMARK=ON QL_BUILD_TEST_SUITE=ON
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

if [[ $? -eq 0 ]]; then
    echo "Build succeeded."
else
    echo "Build failed."
fi
