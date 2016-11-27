project(HelloQuantLib)
cmake_minimum_required(VERSION 2.8.12)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

find_package("QuantLib")

if (QuantLib_FOUND)
	if(MSVC)
		add_definitions(-DUNICODE                     # Use unicode
						-D_UNICODE                    # Use unicode
						-D_SCL_SECURE_NO_WARNINGS     # Calling any one of the potentially unsafe methods in the Standard C++ Library
						-D_CRT_SECURE_NO_DEPRECATE    # Disable CRT deprecation warnings
						-DBOOST_ALL_NO_LIB			  # Prevent auto-link of Boost libs such as serialization
						)
	endif()

    add_executable(greet main.cpp)
    include_directories(${QuantLib_INCLUDE_DIRS})
    target_link_libraries(greet ${QuantLib_LIBRARIES})
else()
    message(FATAL_ERROR "QuantLib not found")
endif()

