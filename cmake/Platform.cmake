# Platform-specific flags and settings
if (MSVC)
    # See cmake policy CMP00091
    # One of "MultiThreaded", "MultiThreadedDebug", "MultiThreadedDLL", "MultiThreadedDebugDLL"
    if (NOT DEFINED CMAKE_MSVC_RUNTIME_LIBRARY)
        set(CMAKE_MSVC_RUNTIME_LIBRARY
            "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    endif()

    # Export all symbols so MSVC can populate the .lib and .dll
    if (BUILD_SHARED_LIBS)
        # Temp: disable DLL builds on MSVC
        message(FATAL_ERROR
            "Shared library (DLL) builds for QuantLib on MSVC are not supported")
        set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
    endif()

    add_compile_definitions(NOMINMAX)

    # caused by ql\time\date.cpp: warning C4996: 'localtime': This function or variable may be unsafe. Consider using localtime_s instead.
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)

    # /wd4267
    # Suppress warnings: assignment of 64-bit value to 32-bit QuantLib::Integer (x64)

    # /wd4819
    # Suppress warnings: The file contains a character that cannot be represented in the current code page

    # /wd26812
    # Suppress warnings: "Prefer enum class over enum" (Enum.3)

    add_compile_options(/wd4267 /wd4819 /wd26812)

    # prevent warnings when using /std:c++17 and above
    if(CMAKE_CXX_STANDARD GREATER_EQUAL 17)
        # E.g. caused by #include <boost/numeric/ublas/matrix.hpp>
        # In c++17 std::iterator is deprecated. As of boost 1.81 boost::ublas has not provided a fix for this.
        add_compile_definitions(_SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING)
    endif()
endif()
