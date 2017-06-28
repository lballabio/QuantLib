file(GLOB_RECURSE QUANTLIB_FILES "*.hpp" "*.cpp")

add_library(QuantLib STATIC ${QUANTLIB_FILES})
if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    set_target_properties(QuantLib PROPERTIES OUTPUT_NAME "QuantLib-vc140-x64-mt-gd")
elseif(${CMAKE_BUILD_TYPE} STREQUAL "Release")
    set_target_properties(QuantLib PROPERTIES OUTPUT_NAME "QuantLib-vc140-x64-mt")
endif()
