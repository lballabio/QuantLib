file(GLOB_RECURSE QUANTLIB_FILES "*.hpp" "*.cpp")
add_library(QuantLib SHARED ${QUANTLIB_FILES})
