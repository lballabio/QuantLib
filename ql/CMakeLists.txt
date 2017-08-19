file(GLOB_RECURSE QUANTLIB_FILES "*.hpp" "*.cpp")
add_library(QuantLib SHARED ${QUANTLIB_FILES})

install(DIRECTORY . DESTINATION include/QuantLib
        FILES_MATCHING PATTERN "*.hpp" PATTERN "*.h")

install(TARGETS QuantLib
	LIBRARY DESTINATION lib
)
