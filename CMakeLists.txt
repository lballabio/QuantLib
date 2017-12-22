cmake_minimum_required()

project(QuantLib)

include(${CMAKE_CURRENT_LIST_DIR}/cmake/quantlib.cmake)

option(BUILD_SHARED_LIBS "Build shared libraries" ${UNIX})
option(USE_BOOST_DYNAMIC_LIBRARIES "Use the shared version of Boost libraries" ${UNIX})
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

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/bin)

link_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib)
link_directories($ENV{BOOST_ROOT}/stage/lib)

find_package(Boost)
if (Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
endif (Boost_FOUND)

add_subdirectory(ql)
add_subdirectory(Examples)
add_subdirectory(test-suite)
