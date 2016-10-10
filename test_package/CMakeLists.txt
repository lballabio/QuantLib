project(HelloQuantLib)
cmake_minimum_required(VERSION 2.8.12)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

find_package("Quantlib")

if (QUANTLIB_FOUND)
    add_executable(greet main.cpp)
    include_directories(${QUANTLIB_INCLUDE_DIRS})
    target_link_libraries(greet ${QUANTLIB_LIBRARIES})
else()
    message(FATAL_ERROR "Quantlib not found")
endif()

