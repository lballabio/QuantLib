file(GLOB_RECURSE QUANTLIB_FILES "*.hpp" "*.cpp")
add_library(quantlib SHARED ${QUANTLIB_FILES})
