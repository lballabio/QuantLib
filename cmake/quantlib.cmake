cmake_minimum_required(VERSION 3.0)  # Required for CMAKE_VS_PLATFORM_TOOLSET

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
        elseif(NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 15)
            set(QL_LIB_TOOLSET "-vc90")
        elseif(NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 14)
            set(QL_LIB_TOOLSET "-vc80")
        elseif(NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 13.10)
            set(QL_LIB_TOOLSET "-vc71")
        elseif(NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 13) # Good luck!
            set(QL_LIB_TOOLSET "-vc7") # yes, this is correct
        else() # MSVC60 Good luck!
            set(QL_LIB_TOOLSET "-vc6") # yes, this is correct
        endif()
        message(STATUS " - Toolset: ${QL_LIB_TOOLSET}")
        
        # - platform
        if("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
            set(QL_LIB_PLATFORM "-x64")
        endif()
        message(STATUS " - Platform: ${QL_LIB_PLATFORM}")

        # - thread linkage
        set(QL_LIB_THREAD_OPT "-mt")  # _MT is defined for /MD and /MT runtimes (https://docs.microsoft.com/es-es/cpp/build/reference/md-mt-ld-use-run-time-library)
        message(STATUS " - Thread linkage: ${QL_LIB_THREAD_OPT}")
        
        # - release/debug is selected during build, so it cannot be part of the generated name
        set(CMAKE_DEBUG_POSTFIX "-gd")

        set(${QL_OUTPUT_NAME} "QuantLib${QL_LIB_TOOLSET}${QL_LIB_PLATFORM}${QL_LIB_THREAD_OPT}")
    else()
        set(${QL_OUTPUT_NAME} "QuantLib")
    endif()
    message(STATUS "QuantLib library name: ${${QL_OUTPUT_NAME}}")
endmacro(get_quantlib_library_name)
