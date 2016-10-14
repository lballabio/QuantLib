cmake_minimum_required()

option(BUILD_EXAMPLES "Build examples" ON)
option(BUILD_TESTS "Build tests" ON)
# to reference headers via <ql/foo.hpp>, we need to add the root
# directory of the project to includes
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

find_package(Boost)
if (Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
endif (Boost_FOUND)

if (BUILD_EXAMPLES)
    add_subdirectory(Examples)
endif (BUILD_EXAMPLES)
add_subdirectory(ql)

# Building the test suite might require using shared libraries
# for Boost.Test
option(USE_BOOST_DYNAMIC_LIBRARIES
       "Use the shared version of Boost.Test" OFF)
if (BUILD_TESTS)
    add_subdirectory(test-suite)
endif (BUILD_TESTS)
