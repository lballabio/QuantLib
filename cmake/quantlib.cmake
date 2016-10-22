cmake_minimum_required(VERSION 3.0)  # Required for CMAKE_VS_PLATFORM_TOOLSET

macro(get_quantlib_library_name QL_OUTPUT_NAME)
    message(STATUS "QuantLib library name tokens:")
    
    # MSVC: Give QuantLib built library different names following code in 'ql/autolink.hpp'
    if(MSVC)
        
        # - toolset
        if(CMAKE_VS_PLATFORM_TOOLSET)
            set(QL_LIB_TOOLSET "-${CMAKE_VS_PLATFORM_TOOLSET}")
        else()
            # ...taken from FindBoost.cmake
            if (NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 16)
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
        endif()
        message(STATUS " - Toolset: ${QL_LIB_TOOLSET}")
        
        # - platform
        if("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
            set(QL_LIB_PLATFORM "-x64")
        endif()
        message(STATUS " - Platform: ${QL_LIB_PLATFORM}")

        # - thread linkage
        if (CMAKE_CXX_FLAGS MATCHES "/MT")
            set(QL_LIB_THREAD_OPT "-mt")
        endif()
        message(STATUS " - Thread linkage: ${QL_LIB_THREAD_OPT}")
        
        # - abi tags
        if (BUILD_SHARED_LIBS)
            set(CMAKE_DEBUG_POSTFIX "-gd")
        else()
            set(QL_LIB_ABI_TAG "-s")
            set(CMAKE_DEBUG_POSTFIX "gd")
        endif()
        message(STATUS " - ABI tag: ${QL_LIB_ABI_TAG}")
        
        # - release/debug is selected during build, so it cannot be part of the generated name
        
        # - version string
        find_path(QUANTLIB_INCLUDE_DIR NAMES ql/version.hpp PATHS ${CONAN_INCLUDE_DIRS_QUANTLIB} ${QUANTLIB_ROOT} ${CMAKE_SOURCE_DIR})
        file(STRINGS "${QUANTLIB_INCLUDE_DIR}/ql/version.hpp" _quantlib_VERSION_HPP_CONTENTS REGEX "#define QL_LIB_VERSION \"([0-9_]+)\"")
        if("${_quantlib_VERSION_HPP_CONTENTS}" MATCHES "#define QL_LIB_VERSION \"([0-9_]+)\"")
            set(QL_LIB_VERSION "-${CMAKE_MATCH_1}")
        endif()
        unset(_quantlib_VERSION_HPP_CONTENTS)
        message(STATUS " - Version string: ${QL_LIB_VERSION}")
                
        set(${QL_OUTPUT_NAME} "QuantLib${QL_LIB_TOOLSET}${QL_LIB_PLATFORM}${QL_LIB_THREAD_OPT}${QL_LIB_VERSION}${QL_LIB_ABI_TAG}")
    else()
        set(${QL_OUTPUT_NAME} "QuantLib")
    endif()
    message(STATUS "QuantLib library name: ${${QL_OUTPUT_NAME}}")
endmacro(get_quantlib_library_name)

