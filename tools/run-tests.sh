#!/bin/bash

BUILD_TYPE=Release
CMAKE_PRESET=linux-base
TEST_SUITE=""

help() {
    echo "run-tests.sh - run the QuantLib library tests (or a subset of the tests)
    Usage: run-tests.sh [options] [positional arguments]
    
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
        ./run-tests.sh -bt Release -p linux-base QL_BUILD_BENCHMARK=OFF
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
    BUILD_ARGS=""
    
    if [[ -n $BUILD_TYPE ]]; then
        BUILD_ARGS="-bt $BUILD_TYPE"
    fi

    if [[ -n $CMAKE_PRESET ]]; then
        BUILD_ARGS="$BUILD_ARGS -p $CMAKE_PRESET"
    fi

    source $BASEDIR/tools/build-with-cmake.sh $BUILD_ARGS "${POSITIONAL_ARGS[@]}"
fi

CORES=$((`nproc`))

pushd $BUILD_DIR

# Rebuild changed files
make -j$CORES

pushd test-suite

./quantlib-test-suite --log_level=message --run_test=*/$TEST_SUITE

popd

popd

if [[ $? -eq 0 ]]; then
    echo "Testing suit succeeded."
else
    echo "Testing suit failed."
fi
