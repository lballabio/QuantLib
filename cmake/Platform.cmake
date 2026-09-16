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

    # /wd4267
    # Suppress warnings: assignment of 64-bit value to 32-bit QuantLib::Integer (x64).
    # There are several hundred such narrowing conversions, most of them at call
    # boundaries into public Integer/Natural parameters, so removing them would
    # mean changing the public API.
    # The Visual Studio projects get the same suppression from the #pragma in
    # ql/config.msvc.hpp instead; a cmake build never includes that header, since
    # CMakeLists.txt sets QL_HAVE_CONFIG_H and thus selects the generated
    # ql/config.hpp.  The two are not redundant.

    # /wd4819
    # Suppress warnings: The file contains a character that cannot be represented in the current code page

    # /wd26812
    # Suppress warnings: "Prefer enum class over enum" (Enum.3)

    # /w34127
    # Enable warning under W3: conditional expression is constant

    # /w34702
    # Enable warning under W3: unreachable code

    # /w35262
    # Enable warning under W3: implicit fall-through occurs here; are you missing a break statement?

    add_compile_options(/wd4267 /wd4819 /wd26812 /w34127 /w34702 /w35262)

    # Two C++17 deprecations reach us from boost, which we still support back
    # to 1.58:
    #  - std::iterator, which older boost::ublas derives its iterators from
    #    (included e.g. through ql/math/matrixutilities/sparsematrix.hpp);
    #  - the old std::allocator members, which older boost::unordered uses
    #    (exercised by test-suite/timeseries.cpp).
    # Silence those two rather than all of them: a blanket
    # _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS would also hide any deprecated
    # construct we introduced ourselves.
    add_compile_definitions(
        _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING
        _SILENCE_CXX17_OLD_ALLOCATOR_MEMBERS_DEPRECATION_WARNING)
endif()
