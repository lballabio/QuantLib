#!/bin/bash

BUILD_TYPE=Release
CMAKE_PRESET=linux-base
TEST_SUITE=""

help() {
    echo "run-tests.sh -- Run QuantLib tests in the cmake folder
                   [-bt|--build-type] - the type of build (Debug/Release)
                   [-p|--preset] - cmake-preset to use
                   [-ts|--test-suite] - test suite to run
                   [-h|--help] - Output the help message 
    "
    exit 1
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
        -ts|--test-suite)
            TEST_SUITE="$2/*"
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
    exit 1
fi

BUILD_DIR=$BASEDIR/build/$CMAKE_PRESET

if [ ! -d $BUILD_DIR ]
then
    source $BASEDIR/tools/build-with-cmake.sh -bt $BUILD_TYPE 
fi

CORES=$((`nproc`))

pushd $BUILD_DIR

# Rebuild changed files
make -j$CORES

pushd test-suite

./quantlib-test-suite --log_level=message --run_test=*/$TEST_SUITE

popd

popd

# Echo results of make
if [[ $? -eq 0 ]]; then
    echo "Testing suit succeeded."
else
    echo "Testing suit failed."
fi
