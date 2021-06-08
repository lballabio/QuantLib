cmake_minimum_required()

project(QuantLib)

if(APPLE AND NOT CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 11)
endif()

include(CTest)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/quantlib.cmake)

option(BUILD_SHARED_LIBS "Build shared libraries" ${UNIX})
option(USE_BOOST_DYNAMIC_LIBRARIES "Use the shared version of Boost libraries" ${UNIX})

option(BUILD_TESTS "Build the tests" ON)
option(BUILD_EXAMPLES "Build the examples" ON)

if (USE_BOOST_DYNAMIC_LIBRARIES)
    add_definitions(-DBOOST_ALL_DYN_LINK)
else()
    set(Boost_USE_STATIC_LIBS ON)
endif()

if (MSVC)
    set(MSVC_RUNTIME "dynamic" CACHE STRING "MSVC runtime to link")
    set_property(CACHE MSVC_RUNTIME PROPERTY STRINGS static dynamic)
    configure_msvc_runtime()

    if(${MSVC_RUNTIME} STREQUAL "static")
        if(USE_BOOST_DYNAMIC_LIBRARIES)
            message(FATAL_ERROR "Use of shared Boost libraries while compiling with static runtime seems not be a good idea.")
        endif()
        set(Boost_USE_STATIC_RUNTIME ON)
    endif()

    # Remove warnings
    add_definitions(-D_SCL_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS)
endif()

# On single-configuration builds, select a default build type that
# gives the same compilation flags as a default autotools build.
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "RelWithDebInfo")
endif()

# to reference headers via <ql/foo.hpp>, we need to add the root
# directory of the project to includes
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

find_package(Boost 1.48.0)
if (Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
endif (Boost_FOUND)

add_subdirectory(ql)
if (BUILD_EXAMPLES)
    add_subdirectory(Examples)
endif()
if (BUILD_TESTS)
    add_subdirectory(test-suite)
endif()


option(QL_ERROR_FUNCTIONS "Define this if error messages should include current function information." OFF)
option(QL_ERROR_LINES "Define this if error messages should include file and line information." OFF)
option(QL_ENABLE_TRACING "Define this if tracing messages should be allowed (whether they are actually emitted will depend on run-time settings)." OFF)
option(QL_EXTRA_SAFETY_CHECKS "Define this if extra safety checks should be performed. This can degrade performance." OFF)
option(QL_USE_INDEXED_COUPON "Define this to use indexed coupons instead of par coupons in floating legs as the default in 'static bool IborCoupon::usingAtParCoupons();'" OFF)
option(QL_ENABLE_SESSIONS "Define this to have singletons return different instances for different sessions. You will have to provide and link with the library
a sessionId() function in namespace QuantLib, returning a different session id for each session." OFF)

option(QL_ENABLE_THREAD_SAFE_OBSERVER_PATTERN "Define this to enable the thread-safe observer pattern. You should
enable it if you want to use QuantLib via the SWIG layer within
the JVM or .NET eco system or any environment with an
async garbage collector" OFF)

option(QL_HIGH_RESOLUTION_DATE "Define this to enable a date resolution down to microseconds and
allow for accurate intraday pricing." OFF)

option(QL_USE_STD_SHARED_PTR  "Define this to use standarad smart pointers instead of Boost ones.
Note that std::shared_ptr does not check access and can
cause segmentation faults." OFF)

option(QL_USE_STD_UNIQUE_PTR "Undefine this to use std::auto_ptr instead of std::unique_ptr." ON)
option(QL_USE_STD_FUNCTION "Define this to use std::function and std::bind instead of boost::function and boost::bind." OFF)
option(QL_USE_STD_TUPLE  "Define this to use std::tuple instead of boost::tuple." OFF)
option(QL_USE_DISPOSABLE "Define this if you want to use the Disposable class template.
This should be no longer necessary in C++11
and might interfere with compiler optimizations." OFF)

option(QL_ENABLE_PARALLEL_UNIT_TEST_RUNNER "Define this to enable the parallel unit test runner" OFF)
option(QL_ENABLE_SINGLETON_THREAD_SAFE_INIT "Define this to make Singleton initialization thread-safe.
Note: There is no support for thread safety and multiple sessions." OFF)


#
# Config the ANSI config file into the build directory. Users
# need to change userconfig.hpp if they require different settings.
#
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/ql/config.ansi.hpp.in ${PROJECT_BINARY_DIR}/config.hpp )
install(FILES ${PROJECT_BINARY_DIR}/config.hpp DESTINATION include/ql)
