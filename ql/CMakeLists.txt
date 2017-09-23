file(GLOB_RECURSE QUANTLIB_FILES "*.hpp" "*.cpp")
add_library(QuantLib SHARED ${QUANTLIB_FILES})
add_library(QuantLib_Static STATIC ${QUANTLIB_FILES})

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    if(NOT CMAKE_CL_64)
        set_target_properties(QuantLib_Static PROPERTIES OUTPUT_NAME "QuantLib-vc140-mt-gd")
    else()
        set_target_properties(QuantLib_Static PROPERTIES OUTPUT_NAME "QuantLib-vc140-x64-mt-gd")
    endif()
    elseif(${CMAKE_BUILD_TYPE} STREQUAL "Release")
    if(NOT CMAKE_CL_64)
        set_target_properties(QuantLib_Static PROPERTIES OUTPUT_NAME "QuantLib-vc140-mt")
    else()
        set_target_properties(QuantLib_Static PROPERTIES OUTPUT_NAME "QuantLib-vc140-x64-mt")
    endif()
endif()

install(DIRECTORY . DESTINATION include/ql
        FILES_MATCHING PATTERN "*.hpp" PATTERN "*.h")

install(TARGETS QuantLib
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
        )

install(TARGETS QuantLib_Static
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
        )
