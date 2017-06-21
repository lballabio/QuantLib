file(GLOB_RECURSE QUANTLIB_FILES "*.hpp" "*.cpp")

add_library(QuantLib STATIC ${QUANTLIB_FILES})
set_target_properties(QuantLib PROPERTIES OUTPUT_NAME "QuantLib-vc140-x64-mt")
