cmake_minimum_required(VERSION 3.0)  # Required for CMAKE_VS_PLATFORM_TOOLSET

macro(get_quantlib_library_name QL_OUTPUT_NAME)
    # MSVC: Give QuantLib built library different names following code in 'ql/autolink.hpp'
    if(MSVC)
        # - toolset
        if(CMAKE_VS_PLATFORM_TOOLSET)
            set(QL_LIB_TOOLSET ${CMAKE_VS_PLATFORM_TOOLSET})
        else()
            string(REGEX REPLACE "^([0-9]+).*$" "\\1" CMAKE_CXX_COMPILER_VERSION_MAJOR ${CMAKE_CXX_COMPILER_VERSION})
            if(CMAKE_CXX_COMPILER_VERSION_MAJOR MATCHES "16")
                set(QL_LIB_TOOLSET "v100")
            else()
                message(FATAL_ERROR "Cannot identify toolset for MSVC. Nor 'CMAKE_VS_PLATFORM_TOOLSET' and '${CMAKE_CXX_COMPILER_VERSION}' != '16.0' (MSVC 2010)")
            endif()
        endif()
        
        # - platform
        if("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
            set(QL_LIB_PLATFORM "-x64")
        endif()

        # - thread linkage
        if (CMAKE_CXX_FLAGS MATCHES "/MT")
            set(QL_LIB_THREAD_OPT "-mt")
        endif()
        
        # - runtime + debug/release
        if (BUILD_SHARED_LIBS)
            if (CMAKE_BUILD_TYPE EQUAL "DEBUG")
                set(QL_LIB_DEBUG "-gd")
            endif()
        else()
            if (CMAKE_BUILD_TYPE EQUAL "DEBUG")
                set(QL_LIB_DEBUG "gd")
            endif()
            set(QL_LIB_RT_OPT "-s")
        endif()
        
        set(${QL_OUTPUT_NAME} "QuantLib-${QL_LIB_TOOLSET}${QL_LIB_PLATFORM}${QL_LIB_THREAD_OPT}${QL_LIB_RT_OPT}${QL_LIB_DEBUG}")
    else()
        set(${QL_OUTPUT_NAME} "QuantLib")
    endif()
endmacro(get_quantlib_library_name)

