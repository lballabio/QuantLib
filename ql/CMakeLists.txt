file(GLOB_RECURSE QUANTLIB_FILES "*.hpp" "*.cpp")

include(${CMAKE_CURRENT_LIST_DIR}/../cmake/quantlib.cmake)
get_quantlib_library_name(QL_OUTPUT_NAME)
message("QuantLib library name: ${QL_OUTPUT_NAME}[${CMAKE_DEBUG_POSTFIX}]")

set(BUILD_SHARED_LIBS OFF)
add_library(QuantLib ${QUANTLIB_FILES})
set_target_properties(QuantLib PROPERTIES OUTPUT_NAME ${QL_OUTPUT_NAME})

