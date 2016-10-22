project(HelloQuantLib)
cmake_minimum_required(VERSION 2.8.12)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

find_package("QuantLib")

if (QuantLib_FOUND)
    add_executable(greet main.cpp)
    include_directories(${QuantLib_INCLUDE_DIRS})
    target_link_libraries(greet ${QuantLib_LIBRARIES})
else()
    message(FATAL_ERROR "QuantLib not found")
endif()

