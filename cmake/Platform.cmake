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

    # /utf-8
    # The sources are UTF-8 (author names in the licence headers, mathematical
    # symbols in the documentation).  Tell MSVC so, rather than suppressing
    # C4819 "The file contains a character that cannot be represented in the
    # current code page".  There are no non-ASCII narrow string literals left,
    # so setting the execution character set along with the source one has no
    # effect on the generated code.

    # /wd26812
    # Suppress warnings: "Prefer enum class over enum" (Enum.3)

    # /w34127
    # Enable warning under W3: conditional expression is constant

    # /w34702
    # Enable warning under W3: unreachable code

    # /w35262
    # Enable warning under W3: implicit fall-through occurs here; are you missing a break statement?

    add_compile_options(/wd4267 /utf-8 /wd26812 /w34127 /w34702 /w35262)

    # Silence all C++17 deprecation warnings, same as in the vcxproj files
    add_compile_definitions(_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS)
endif()
