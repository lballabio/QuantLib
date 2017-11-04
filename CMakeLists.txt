cmake_minimum_required()

project(QuantLib)

# to reference headers via <ql/foo.hpp>, we need to add the root
# directory of the project to includes
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

find_package(Boost)
if (Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
endif (Boost_FOUND)

add_subdirectory(ql)
add_subdirectory(Examples)

# Building the test suite might require using shared libraries
# for Boost.Test
option(USE_BOOST_DYNAMIC_LIBRARIES
       "Use the shared version of Boost.Test" ON)
add_subdirectory(test-suite)
