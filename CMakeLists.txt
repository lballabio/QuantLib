cmake_minimum_required()

# to reference headers via <ql/foo.hpp> we need to add root proj dir to includes
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(Examples)
add_subdirectory(ql)
add_subdirectory(test-suite)