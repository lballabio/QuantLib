cmake_minimum_required(VERSION 3.15.0)

# For MSVC RUNTIME LIBRARY, need CMP0091=NEW and cmake 3.15+
cmake_policy(SET CMP0091 NEW)

# Version info
set(QUANTLIB_VERSION_MAJOR 1)
set(QUANTLIB_VERSION_MINOR 27)
set(QUANTLIB_VERSION_PATCH 0)
set(QUANTLIB_VERSION ${QUANTLIB_VERSION_MAJOR}.${QUANTLIB_VERSION_MINOR}.${QUANTLIB_VERSION_PATCH})

# Project Info
set(PACKAGE_NAME          "QuantLib")
set(PACKAGE_VERSION       "${QUANTLIB_VERSION}-dev")
set(PACKAGE_VERSION_HEX   "0x01270000")
set(PACKAGE_STRING        "${PACKAGE_NAME} ${PACKAGE_VERSION}")
set(PACKAGE_TARNAME       "${PACKAGE_NAME}-${PACKAGE_VERSION}")
set(PACKAGE_BUGREPORT     "https://github.com/lballabio/QuantLib/issues/")

# Default build type for single-config generators (set this before project() command)
# For multi-config generators, such as Visual Studio, use: cmake --build . --config=<CONFIG>
if (NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "")
    set(CMAKE_BUILD_TYPE "Release" CACHE
        STRING "Build type for single-config generators" FORCE)
endif()

project(${PACKAGE_NAME} LANGUAGES CXX DESCRIPTION "The QuantLib C++ Library")

# Path for package-local cmake modules
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

# Installation directories
set(QL_INSTALL_BINDIR "bin" CACHE STRING "Installation directory for executables")
set(QL_INSTALL_LIBDIR "lib" CACHE STRING "Installation directory for libraries")
set(QL_INSTALL_INCLUDEDIR "include" CACHE STRING "Installation directory for headers")
set(QL_INSTALL_EXAMPLESDIR "lib/QuantLib/examples" CACHE STRING
    "Installation directory for examples")
set(QL_INSTALL_CMAKEDIR "lib/cmake/${PACKAGE_NAME}" CACHE STRING
    "Installation directory for CMake scripts")

# Options
option(QL_BUILD_BENCHMARK "Build benchmark" ON)
option(QL_BUILD_EXAMPLES "Build examples" ON)
option(QL_BUILD_TEST_SUITE "Build test suite" ON)
option(QL_ENABLE_OPENMP "Detect and use OpenMP" OFF)
option(QL_ENABLE_PARALLEL_UNIT_TEST_RUNNER "Enable the parallel unit test runner" OFF)
option(QL_ENABLE_SESSIONS "Singletons return different instances for different sessions" OFF)
option(QL_ENABLE_SINGLETON_THREAD_SAFE_INIT "Enable thread-safe singleton initialization" OFF)
option(QL_ENABLE_THREAD_SAFE_OBSERVER_PATTERN "Enable the thread-safe observer pattern" OFF)
option(QL_ENABLE_TRACING "Tracing messages should be allowed" OFF)
option(QL_ERROR_FUNCTIONS "Error messages should include current function information" OFF)
option(QL_ERROR_LINES "Error messages should include file and line information" OFF)
option(QL_EXTRA_SAFETY_CHECKS "Extra safety checks should be performed" OFF)
option(QL_HIGH_RESOLUTION_DATE "Enable date resolution down to microseconds" OFF)
option(QL_INSTALL_BENCHMARK "Install benchmark" ON)
option(QL_INSTALL_EXAMPLES "Install examples" ON)
option(QL_INSTALL_TEST_SUITE "Install test suite" ON)
option(QL_TAGGED_LAYOUT "Library names use layout tags" ${MSVC})
option(QL_USE_CLANG_TIDY "Use clang-tidy when building" OFF)
option(QL_USE_INDEXED_COUPON "Use indexed coupons instead of par coupons" OFF)
option(QL_USE_STD_CLASSES "Enable all QL_USE_STD_ options" OFF)
option(QL_USE_STD_SHARED_PTR "Use standard smart pointers instead of Boost ones" OFF)
option(QL_USE_STD_FUNCTION "Use std::function and std::bind instead of Boost ones" OFF)
option(QL_USE_STD_TUPLE "Use std::tuple instead of boost::tuple" OFF)

set(QL_USE_AAD "None" CACHE STRING "Use AAD with the given framework (options: None, XAD)")
# set available drop-down options for AAD tool
set(_aad_frameworks XAD)
set_property(CACHE QL_USE_AAD PROPERTY STRINGS None ${_aad_frameworks})



# Convenience option to activate all STD options
if (QL_USE_STD_CLASSES)
    set(QL_USE_STD_SHARED_PTR ON)
    set(QL_USE_STD_FUNCTION ON)
    set(QL_USE_STD_TUPLE ON)
endif()

if (QL_USE_CLANG_TIDY)
    if (NOT DEFINED QL_CLANG_TIDY)
        set(QL_CLANG_TIDY clang-tidy)
    endif()
    if (NOT DEFINED QL_CLANG_TIDY_OPTIONS)
        set(QL_CLANG_TIDY_OPTIONS)
    endif()
    if (QL_CLANG_TIDY_OPTIONS)
        set(CMAKE_CXX_CLANG_TIDY "${QL_CLANG_TIDY};${QL_CLANG_TIDY_OPTIONS}")
    else()
        set(CMAKE_CXX_CLANG_TIDY "${QL_CLANG_TIDY}")
    endif()
endif()

# Project shared libs ON for UNIX
if (NOT DEFINED BUILD_SHARED_LIBS)
    set(BUILD_SHARED_LIBS ${UNIX})
endif()

# Boost static libs ON for MSVC
if (NOT DEFINED Boost_USE_STATIC_LIBS)
    set(Boost_USE_STATIC_LIBS ${MSVC})
endif()

# Boost static runtime ON for MSVC
if (NOT DEFINED Boost_USE_STATIC_RUNTIME)
    set(Boost_USE_STATIC_RUNTIME ${MSVC})
endif()

# Require C++11 or higher
if (NOT DEFINED CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 11)
elseif(CMAKE_CXX_STANDARD LESS 11)
    message(FATAL_ERROR "Please specify CMAKE_CXX_STANDARD of 11 or higher")
endif()
if (NOT DEFINED CMAKE_CXX_STANDARD_REQUIRED)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()
# Avoid use of compiler language extensions, i.e. -std=c++11 not -std=gnu++11
if (NOT DEFINED CMAKE_CXX_EXTENSIONS)
    set(CMAKE_CXX_EXTENSIONS FALSE)
endif()

if (NOT DEFINED QL_BOOST_VERSION)
    # Boost 1.75.0 or greater required for compiling with C++20
    if (CMAKE_CXX_STANDARD GREATER_EQUAL 20)
        set(QL_BOOST_VERSION 1.75.0)
    else()
        set(QL_BOOST_VERSION 1.58.0)
    endif()
endif()

if (NOT DEFINED QL_BOOST_COMPONENTS)
    set(QL_BOOST_COMPONENTS "")
    # Boost thread needed for sessions, singleton thread-safe init and thread-safe observer pattern
    if(QL_ENABLE_SESSIONS OR QL_ENABLE_SINGLETON_THREAD_SAFE_INIT OR QL_ENABLE_THREAD_SAFE_OBSERVER_PATTERN)
        set(QL_BOOST_COMPONENTS "${QL_BOOST_COMPONENTS};thread")
    endif()
endif()

find_package(Boost ${QL_BOOST_VERSION} REQUIRED COMPONENTS ${QL_BOOST_COMPONENTS})

# Do not warn about Boost versions higher than 1.58.0
set(Boost_NO_WARN_NEW_VERSIONS ON)

# Avoid using Boost auto-linking
add_compile_definitions(BOOST_ALL_NO_LIB)

if (QL_ENABLE_OPENMP)
    find_package(OpenMP REQUIRED)
endif()

# Prefer pthread flag as per https://cmake.org/cmake/help/latest/module/FindThreads.html
if (NOT DEFINED THREADS_PREFER_PTHREAD_FLAG)
    set(THREADS_PREFER_PTHREAD_FLAG ON)
endif()

set(QL_THREAD_LIBRARIES)

if (QL_ENABLE_PARALLEL_UNIT_TEST_RUNNER)
    find_package(Threads REQUIRED)
    set(QL_THREAD_LIBRARIES Threads::Threads ${QL_THREAD_LIBRARIES})
    # Parallel test runner needs library rt on *nix for shm_open, etc.
    if (UNIX AND NOT APPLE)
        find_library(RT_LIBRARY rt REQUIRED)
        set(QL_THREAD_LIBRARIES ${RT_LIBRARY} ${QL_THREAD_LIBRARIES})
    endif()
endif()

if (QL_ENABLE_SESSIONS OR QL_ENABLE_SINGLETON_THREAD_SAFE_INIT OR QL_ENABLE_THREAD_SAFE_OBSERVER_PATTERN)
    set(QL_THREAD_LIBRARIES Boost::thread ${QL_THREAD_LIBRARIES})
endif()

# If available, use PIC for shared libs and PIE for executables
if (NOT DEFINED CMAKE_POSITION_INDEPENDENT_CODE)
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()
if (CMAKE_POSITION_INDEPENDENT_CODE)
    # cmake policy CMP0083: add PIE support if possible (need cmake 3.14)
    include(CheckPIESupported)
    check_pie_supported()
endif()

# Configure files
set(QL_HAVE_CONFIG_H ON)
set(QL_VERSION ${PACKAGE_VERSION})
set(QL_HEX_VERSION ${PACKAGE_VERSION_HEX})
configure_file(ql/config.hpp.cfg ql/config.hpp @ONLY)
configure_file(ql/qldefines.hpp.cfg ql/qldefines.hpp @ONLY)
configure_file(ql/version.hpp.cfg ql/version.hpp @ONLY)

# Check for library name layout tagging
if (QL_TAGGED_LAYOUT)
    if (${CMAKE_SIZEOF_VOID_P} EQUAL 8)
        set(DEBUG_POSTFIX "-x64")
        set(RELEASE_POSTFIX "-x64")
    endif()
    set(DEBUG_POSTFIX ${DEBUG_POSTFIX}-mt)
    set(RELEASE_POSTFIX ${RELEASE_POSTFIX}-mt)
    if (NOT BUILD_SHARED_LIBS)
        set(DEBUG_POSTFIX ${DEBUG_POSTFIX}-sgd)
        set(RELEASE_POSTFIX ${RELEASE_POSTFIX}-s)
    else()
        set(DEBUG_POSTFIX ${DEBUG_POSTFIX}-gd)
    endif()
    set(CMAKE_DEBUG_POSTFIX ${DEBUG_POSTFIX})
    set(CMAKE_RELEASE_POSTFIX ${RELEASE_POSTFIX})
    set(CMAKE_RELWITHDEBINFO_POSTFIX ${RELEASE_POSTFIX})
    set(CMAKE_MINSIZEREL_POSTFIX ${RELEASE_POSTFIX})
endif()

include(CTest)
include(Platform)

# handle AAD framework download / integration
if("${QL_USE_AAD}" STREQUAL "XAD")
    message(STATUS "Using AAD with framework ${QL_USE_AAD}")

    # we fetch from git into the source tree, only if those folders are not
    # already existing. This is to allow users to clone the repos themselves,
    # possibly using a fork.
    
    # XAD
    if(NOT EXISTS ${PROJECT_SOURCE_DIR}/xad OR NOT EXISTS ${PROJECT_SOURCE_DIR}/xad/CMakeLists.txt)
        message(STATUS "Did not find XAD in source tree - attempting to clone")
        find_package(Git REQUIRED)
        execute_process(
            COMMAND ${GIT_EXECUTABLE} clone https://github.com/xcelerit/xad.git --branch main ${PROJECT_SOURCE_DIR}/xad
            RESULT_VARIABLE _clone_success
        )    
        if(NOT "${_clone_success}" EQUAL 0)
            message(FATAL_ERROR "Error cloning XAD repository - please clone https://github.com/xcelerit/xad.git manually into xad/ folder")
        endif()
    endif()
    # QuantLib uses static runtime in MSVC
    set(XAD_STATIC_MSVC_RUNTIME ${MSVC} CACHE BOOL "" FORCE)  
    add_subdirectory(xad)

    # Aaptor module with tests
    if(NOT EXISTS ${PROJECT_SOURCE_DIR}/qlxad OR NOT EXISTS ${PROJECT_SOURCE_DIR}/qlxad/CMakeLists.txt)
        message(STATUS "Did not find qlxad in source tree - attempting to clone")
        find_package(Git REQUIRED)
        execute_process(
            COMMAND ${GIT_EXECUTABLE} clone https://github.com/xcelerit/qlxad.git --branch main ${PROJECT_SOURCE_DIR}/qlxad
            RESULT_VARIABLE _clone_success
        )    
        if(NOT "${_clone_success}" EQUAL 0)
            message(FATAL_ERROR "Error cloning qlxad repository - please clone https://github.com/xcelerit/qlxad.git manually into qlxad/ folder")
        endif()
    endif()
    add_subdirectory(qlxad)
else()
    message(STATUS "Not using AAD")
endif()

# Add subdirectories
add_subdirectory(ql)
if (QL_BUILD_EXAMPLES)
    add_subdirectory(Examples)
endif()
if (QL_BUILD_TEST_SUITE OR QL_BUILD_BENCHMARK)
    add_subdirectory(test-suite)
endif()

# CPack support (make package, make package_source)
set(CPACK_PACKAGE_VERSION_MAJOR ${QUANTLIB_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${QUANTLIB_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${QUANTLIB_VERSION_PATCH})
set(CPACK_PACKAGE_FILE_NAME ${PACKAGE_NAME}-${PACKAGE_VERSION})
set(CPACK_GENERATOR "TGZ" "ZIP" "7Z")
set(CPACK_SOURCE_PACKAGE_FILE_NAME ${PACKAGE_NAME}-${PACKAGE_VERSION})
set(CPACK_SOURCE_GENERATOR "TGZ" "ZIP" "7Z")
set(CPACK_SOURCE_IGNORE_FILES
    "~$"
    "\\\\.swp$"
    "/[Bb]uild"
    "/\\\\.app"
    "/\\\\.cla"
    "/\\\\.cod"
    "/\\\\.git"
    "/\\\\.lgt"
    "/\\\\.mis")
include(CPack)
