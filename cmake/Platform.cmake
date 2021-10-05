# Platform-specific flags and settings
if (MSVC)
    # See cmake policy CMP00091
    # One of "MultiThreaded", "MultiThreadedDebug", "MultiThreadedDLL", "MultiThreadedDebugDLL"
    set(CMAKE_MSVC_RUNTIME_LIBRARY
        "MultiThreaded$<$<CONFIG:Debug>:Debug>$<$<BOOL:${BUILD_SHARED_LIBS}>:DLL>")

    # Export all symbols so MSVC can populate the .lib and .dll
    if (BUILD_SHARED_LIBS)
        # Temp: disable DLL builds on MSVC
        message(FATAL_ERROR
            "Shared library (DLL) builds for QuantLib on MSVC are not supported")
        set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
    endif()

    add_compile_definitions(NOMINMAX)

    # /wd4267
    # Suppress warnings: assignment of 64-bit value to 32-bit QuantLib::Integer (x64)

    # /wd26812
    # Suppress warnings: "Prefer enum class over enum" (Enum.3)

    add_compile_options(/wd4267 /wd26812)

    if(NOT QL_MSVC_DISABLE_PARALLEL_BUILDS)
        # enable parallel builds
        add_compile_options(/MP)
    endif()

    # warning level 3 and all warnings as errors
    add_compile_options(/W3 /WX)

    ### Avoid level 3 warnings for the time being.
    ### However they should be fixed in the long run.

    # caused by ql\time\date.cpp: warning C4996: 'localtime': This function or variable may be unsafe. Consider using localtime_s instead. 
    add_compile_definitions(_CRT_SECURE_NO_DEPRECATE)

    # caused by macro redefinition 'M_PI' etc. e.g. in ql\methods\finitedifferences\solvers\fdmbackwardsolver.cpp
    add_compile_options(/wd4005)

    # prevent warnings when using /std:c++17
    if(CMAKE_CXX_STANDARD GREATER 14)
        # caused by the QL_ENABLE_THREAD_SAFE_OBSERVER_PATTERN
        add_compile_definitions(BOOST_UNORDERED_USE_ALLOCATOR_TRAITS=2)
        
        # caused by #include <boost/numeric/ublas/matrix.hpp> 
        # in e.g. ql\experimental\finitedifferences\fdmdupire1dop.cpp
        add_compile_definitions(_SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING)

        # caused by ql/math/functional.hpp (line 271) compiling test-suite\distributions.cpp
        add_compile_definitions(_SILENCE_CXX17_ADAPTOR_TYPEDEFS_DEPRECATION_WARNING)
    endif()
endif()
