cmake_minimum_required(VERSION 3.18)
project(qlnb LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(QLNB_BUILD_QUANTLIB
    "Build QuantLib from the parent source tree and link it statically"
    ON)
option(QLNB_THREAD_SAFE_OBSERVER
    "Pass QL_ENABLE_THREAD_SAFE_OBSERVER_PATTERN=ON when building QuantLib (threaded/free-threaded use)"
    OFF)

set(QLNB_QUANTLIB_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/.." CACHE PATH
    "Path to the QuantLib source root")

if (QLNB_BUILD_QUANTLIB)
    # Required for correct Python extension linking / Settings singleton behavior.
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
    set(CMAKE_POSITION_INDEPENDENT_CODE ON CACHE BOOL "" FORCE)
    set(QL_USE_STD_SHARED_PTR ON CACHE BOOL "" FORCE)
    set(QL_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(QL_BUILD_TEST_SUITE OFF CACHE BOOL "" FORCE)
    set(QL_BUILD_FUZZ_TEST_SUITE OFF CACHE BOOL "" FORCE)
    set(QL_INSTALL_BENCHMARK OFF CACHE BOOL "" FORCE)
    set(QL_INSTALL_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(QL_INSTALL_TEST_SUITE OFF CACHE BOOL "" FORCE)
    if (QLNB_THREAD_SAFE_OBSERVER)
        set(QL_ENABLE_THREAD_SAFE_OBSERVER_PATTERN ON CACHE BOOL "" FORCE)
    endif()

    # Keep QuantLib install rules out of the Python wheel component.
    set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME "quantlib_internal")
    add_subdirectory(
        "${QLNB_QUANTLIB_ROOT}"
        "${CMAKE_BINARY_DIR}/quantlib"
        EXCLUDE_FROM_ALL)
    set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME "Unspecified")

    set(QLNB_QUANTLIB_TARGET ql_library)
else()
    find_package(QuantLib REQUIRED)
    set(QLNB_QUANTLIB_TARGET QuantLib::QuantLib)
endif()

find_package(Python 3.10 REQUIRED COMPONENTS Interpreter Development.Module)
find_package(nanobind CONFIG REQUIRED)

nanobind_add_module(
    _qlnb
    STABLE_ABI
    NB_STATIC
    src/module.cpp
    src/bind_time.cpp
    src/bind_curves.cpp
    src/bind_instruments.cpp
    src/bind_pricing.cpp
    src/bind_rates_options.cpp
    src/bind_credit.cpp
    src/bind_cms.cpp
    src/bind_inflation.cpp
    src/bind_experimental.cpp
)

target_link_libraries(_qlnb PRIVATE ${QLNB_QUANTLIB_TARGET})
target_compile_features(_qlnb PRIVATE cxx_std_17)

install(TARGETS _qlnb
    LIBRARY DESTINATION qlnb
    COMPONENT python)
