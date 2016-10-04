cmake_minimum_required()

# to reference headers via <ql/foo.hpp>, we need to add the root
# directory of the project to includes
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

option(BUILD_WITH_CONAN "Build QuantLib using conan.io package manager" OFF)
if(BUILD_WITH_CONAN OR EXISTS "${CMAKE_BINARY_DIR}/conanbuildinfo.cmake")
   set(BUILD_WITH_CONAN ON)
   include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
   conan_basic_setup()
endif()

find_package(Boost)
if (Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
endif (Boost_FOUND)

add_subdirectory(Examples)
add_subdirectory(ql)

# Building the test suite might require using shared libraries
# for Boost.Test
option(USE_BOOST_DYNAMIC_LIBRARIES
       "Use the shared version of Boost.Test" OFF)
add_subdirectory(test-suite)
