file (GLOB TEST_SUITE_FILES "*.hpp" "*.cpp")

# these do not appear in vcproj
list(REMOVE_ITEM TEST_SUITE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/quantlibbenchmark.cpp)

if (USE_BOOST_DYNAMIC_LIBRARIES)
   add_definitions(-DBOOST_TEST_DYN_LINK)
endif (USE_BOOST_DYNAMIC_LIBRARIES)

find_package (Boost COMPONENTS unit_test_framework REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
set (TEST quantlib-test-suite)
add_executable (${TEST} ${TEST_SUITE_FILES})
target_link_libraries (${TEST} QuantLib ${Boost_LIBRARIES})
enable_testing ()
add_test (${TEST} ${TEST})
