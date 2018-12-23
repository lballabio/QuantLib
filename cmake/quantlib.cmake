cmake_minimum_required(VERSION 2.8)

macro(get_quantlib_library_name QL_OUTPUT_NAME)
    message(STATUS "QuantLib library name tokens:")

    # MSVC: Give QuantLib built library different names following code in 'ql/autolink.hpp'
    if(MSVC)
        
        # - toolset
        # ...taken from FindBoost.cmake
        if (NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 19.10)
            set(QL_LIB_TOOLSET "-vc141")
        elseif(NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 19)
            set(QL_LIB_TOOLSET "-vc140")
        elseif(NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 18)
            set(QL_LIB_TOOLSET "-vc120")
        elseif(NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 17)
            set(QL_LIB_TOOLSET "-vc110")
        elseif(NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 16)
            set(QL_LIB_TOOLSET "-vc100")
        else()
            message(FATAL_ERROR "Compiler below VC++2010 is not supported")
        endif()
        message(STATUS " - Toolset: ${QL_LIB_TOOLSET}")
        
        # - platform
        if("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
            set(QL_LIB_PLATFORM "-x64")
        endif()
        message(STATUS " - Platform: ${QL_LIB_PLATFORM}")

        # - thread linkage
        set(QL_LIB_THREAD_OPT "-mt")  # _MT is defined for /MD and /MT runtimes (https://docs.microsoft.com/es-es/cpp/build/reference/md-mt-ld-use-run-time-library)
        message(STATUS " - Thread opt: ${QL_LIB_THREAD_OPT}")
        
        # - static/dynamic linkage
        if(${MSVC_RUNTIME} STREQUAL "static")
            set(QL_LIB_RT_OPT "-s")
            set(CMAKE_DEBUG_POSTFIX "gd")
        else()
            set(CMAKE_DEBUG_POSTFIX "-gd")
        endif()
        message(STATUS " - Linkage opt: ${QL_LIB_RT_OPT}")
        
        set(${QL_OUTPUT_NAME} "QuantLib${QL_LIB_TOOLSET}${QL_LIB_PLATFORM}${QL_LIB_THREAD_OPT}${QL_LIB_RT_OPT}")
    else()
        set(${QL_OUTPUT_NAME} "QuantLib")
    endif()
    message(STATUS "QuantLib library name: ${${QL_OUTPUT_NAME}}[${CMAKE_DEBUG_POSTFIX}]")
endmacro(get_quantlib_library_name)

macro(configure_msvc_runtime)
    # Credit: https://stackoverflow.com/questions/10113017/setting-the-msvc-runtime-in-cmake
    if(MSVC)
        # Default to dynamically-linked runtime.
        if("${MSVC_RUNTIME}" STREQUAL "")
            set(MSVC_RUNTIME "dynamic")
        endif()

        # Set compiler options.
        set(variables
            CMAKE_C_FLAGS_DEBUG
            CMAKE_C_FLAGS_MINSIZEREL
            CMAKE_C_FLAGS_RELEASE
            CMAKE_C_FLAGS_RELWITHDEBINFO
            CMAKE_CXX_FLAGS_DEBUG
            CMAKE_CXX_FLAGS_MINSIZEREL
            CMAKE_CXX_FLAGS_RELEASE
            CMAKE_CXX_FLAGS_RELWITHDEBINFO
            )

        if(${MSVC_RUNTIME} STREQUAL "static")
            message(STATUS "MSVC -> forcing use of statically-linked runtime.")
            foreach(variable ${variables})
                if(${variable} MATCHES "/MD")
                    string(REGEX REPLACE "/MD" "/MT" ${variable} "${${variable}}")
                endif()
            endforeach()
        else()
            message(STATUS "MSVC -> forcing use of dynamically-linked runtime." )
            foreach(variable ${variables})
                if(${variable} MATCHES "/MT")
                    string(REGEX REPLACE "/MT" "/MD" ${variable} "${${variable}}")
                endif()
            endforeach()
        endif()
    endif()
endmacro()
