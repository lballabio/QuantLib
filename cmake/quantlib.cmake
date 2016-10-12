cmake_minimum_required(VERSION 3.0)  # Required for CMAKE_VS_PLATFORM_TOOLSET

macro(get_quantlib_library_name QL_OUTPUT_NAME)
    set(${QL_OUTPUT_NAME} "QuantLib")

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
        
        set(${QL_OUTPUT_NAME} "${QL_OUTPUT_NAME}-${QL_LIB_TOOLSET}${QL_LIB_PLATFORM}${QL_LIB_THREAD_OPT}${QL_LIB_RT_OPT}")
    endif()
endmacro(get_quantlib_library_name)

