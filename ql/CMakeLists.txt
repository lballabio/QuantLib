file(GLOB_RECURSE QUANTLIB_FILES "*.hpp" "*.cpp")
add_library(QuantLib STATIC ${QUANTLIB_FILES})
