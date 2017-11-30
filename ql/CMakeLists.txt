
get_quantlib_library_name(QL_OUTPUT_NAME)


file(GLOB_RECURSE QUANTLIB_FILES "*.hpp" "*.cpp")
if(WIN32)
    # Windows needs to link static library (nothing is declared to export in QuantLib)
    add_library(${QL_OUTPUT_NAME} STATIC ${QUANTLIB_FILES})
    set_property(TARGET ${QL_OUTPUT_NAME} PROPERTY PROJECT_LABEL "QuantLib")
else()
    add_library(${QL_OUTPUT_NAME} SHARED ${QUANTLIB_FILES})
endif()
set(QL_LINK_LIBRARY ${QL_OUTPUT_NAME} PARENT_SCOPE)


install(DIRECTORY . DESTINATION include/ql
        FILES_MATCHING PATTERN "*.hpp" PATTERN "*.h")

install(TARGETS ${QL_OUTPUT_NAME}
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
        )

