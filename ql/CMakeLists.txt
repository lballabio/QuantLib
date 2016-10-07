file(GLOB_RECURSE QUANTLIB_FILES "*.hpp" "*.cpp")

set(QL_OUTPUT_NAME "QuantLib")

# MSVC: Give QuantLib built library different names following code in 'ql/autolink.hpp'
if(MSVC)
    # - toolset
    set(QL_LIB_TOOLSET ${CMAKE_VS_PLATFORM_TOOLSET})
    
    # - platform
    if("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
        set(QL_LIB_PLATFORM "-x64")
    endif()

    # - thread linkage
    if (CMAKE_CXX_FLAGS MATCHES "/MT")
        set(QL_LIB_THREAD_OPT "-mt")
    endif()
    
    # - runtime
    if (BUILD_SHARED_LIBS)
        if(NOT CMAKE_DEBUG_POSTFIX)
            set(CMAKE_DEBUG_POSTFIX -gd)
        endif()
    else()
        if(NOT CMAKE_DEBUG_POSTFIX)
            set(CMAKE_DEBUG_POSTFIX gd)
        endif()
        set(QL_LIB_RT_OPT "-s")
    endif()
    
    # - debug/release
    if(NOT CMAKE_DEBUG_POSTFIX)
        set(CMAKE_DEBUG_POSTFIX gd)
    endif()
    
    set(QL_OUTPUT_NAME "${QL_OUTPUT_NAME}-${QL_LIB_TOOLSET}${QL_LIB_PLATFORM}${QL_LIB_THREAD_OPT}${QL_LIB_RT_OPT}")    
    message("QuantLib library name: ${QL_OUTPUT_NAME}[${CMAKE_DEBUG_POSTFIX}]")
endif()

set(BUILD_SHARED_LIBS OFF)
add_library(QuantLib ${QUANTLIB_FILES})
set_target_properties(QuantLib PROPERTIES OUTPUT_NAME ${QL_OUTPUT_NAME})

