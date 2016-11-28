cmake_minimum_required()

# to reference headers via <ql/foo.hpp>, we need to add the root
# directory of the project to includes
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

option(BUILD_WITH_CONAN "Build QuantLib using conan.io package manager" OFF)
if(BUILD_WITH_CONAN OR EXISTS "${CMAKE_BINARY_DIR}/conanbuildinfo.cmake")
   set(BUILD_WITH_CONAN ON)
   include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
   conan_basic_setup()
endif()

find_package(Boost)
if (Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
endif()

if(MSVC)
	add_definitions(-DUNICODE                     # Use unicode
					-D_UNICODE                    # Use unicode
					-D_SCL_SECURE_NO_WARNINGS     # Calling any one of the potentially unsafe methods in the Standard C++ Library
					-D_CRT_SECURE_NO_DEPRECATE    # Disable CRT deprecation warnings
					-DBOOST_ALL_NO_LIB			  # Prevent auto-link of Boost libs such as serialization
					)
endif()

add_subdirectory(ql)

option(BUILD_EXAMPLES "Build QuantLib examples" ON)
message("Build QuantLib examples: ${BUILD_EXAMPLES}")
if (BUILD_EXAMPLES)
    add_subdirectory(Examples)
endif()

option(BUILD_TESTS "Build QuantLib tests" ON)
message("Build QuantLib tests: ${BUILD_TESTS}")
if (BUILD_TESTS)
    add_subdirectory(test-suite)
endif()

