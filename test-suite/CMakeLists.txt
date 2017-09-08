file (GLOB TEST_SUITE_FILES "*.hpp" "*.cpp")
set(BENCHMARK_FILES "main.cpp" "quantlibbenchmark.cpp" "americanoption.cpp" "asianoptions.cpp" "barrieroption.cpp"
       "basketoption.cpp" "batesmodel.cpp" "convertiblebonds.cpp" "digitaloption.cpp" "dividendoption.cpp"
       "europeanoption.cpp" "fdheston.cpp" "hestonmodel.cpp" "interpolations.cpp" "jumpdiffusion.cpp"
       "marketmodel_smm.cpp" "marketmodel_cms.cpp" "lowdiscrepancysequences.cpp" "quantooption.cpp" "riskstats.cpp"
       "shortratemodels.cpp" "utilities.cpp" "utilities.hpp" "swaptionvolstructuresutilities.hpp")

# these do not appear in vcproj
list(REMOVE_ITEM TEST_SUITE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/quantlibbenchmark.cpp)

if (USE_BOOST_DYNAMIC_LIBRARIES)
   add_definitions(-DBOOST_TEST_DYN_LINK)
endif (USE_BOOST_DYNAMIC_LIBRARIES)

find_package (Boost COMPONENTS unit_test_framework REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
set (TEST quantlib-test-suite)
set (BENCHMARK quantlib-benchmark)
add_executable (${TEST} ${TEST_SUITE_FILES})
target_link_libraries (${TEST} QuantLib ${Boost_LIBRARIES})
add_executable (${BENCHMARK} ${BENCHMARK_FILES})
target_link_libraries (${BENCHMARK} QuantLib ${Boost_LIBRARIES})
enable_testing ()
add_test (${TEST} ${TEST})
