# Determine the flags for fuzzing. Use OSS-Fuzz's configuration if available, otherwise fall back to defaults.
if(DEFINED ENV{LIB_FUZZING_ENGINE})
    set(FUZZING_ENGINE $ENV{LIB_FUZZING_ENGINE})
    set(FUZZING_COMPILE_FLAGS "")
    set(FUZZING_LINK_FLAGS "${FUZZING_ENGINE}")
else()
    set(FUZZING_COMPILE_FLAGS "-fsanitize=fuzzer")
    set(FUZZING_LINK_FLAGS "-fsanitize=fuzzer")
endif()

add_executable(DateParserFuzzer dateparserfuzzer.cpp)
set_target_properties(DateParserFuzzer PROPERTIES
    COMPILE_FLAGS "${FUZZING_COMPILE_FLAGS}"
    LINK_FLAGS "${FUZZING_LINK_FLAGS}"
)
target_link_libraries(DateParserFuzzer ql_library ${QL_THREAD_LIBRARIES})

add_executable(AmericanOptionFuzzer fuzzamericanoption.cpp)
set_target_properties(AmericanOptionFuzzer PROPERTIES
    COMPILE_FLAGS "${FUZZING_COMPILE_FLAGS}"
    LINK_FLAGS "${FUZZING_LINK_FLAGS}"
)
target_link_libraries(AmericanOptionFuzzer ql_library ${QL_THREAD_LIBRARIES})

add_executable(AmortizedBondsFuzzer amortizedbondsfuzzer.cpp)
set_target_properties(AmortizedBondsFuzzer PROPERTIES
    COMPILE_FLAGS "${FUZZING_COMPILE_FLAGS}"
    LINK_FLAGS "${FUZZING_LINK_FLAGS}"
)
target_link_libraries(AmortizedBondsFuzzer ql_library ${QL_THREAD_LIBRARIES})

add_executable(EuropeanOptionFuzzer fuzzeuropeanoption.cpp)
set_target_properties(EuropeanOptionFuzzer PROPERTIES
    COMPILE_FLAGS "${FUZZING_COMPILE_FLAGS}"
    LINK_FLAGS "${FUZZING_LINK_FLAGS}"
)
target_link_libraries(EuropeanOptionFuzzer ql_library ${QL_THREAD_LIBRARIES})

add_executable(InterpolationsFuzzer fuzzinterpolations.cpp)
set_target_properties(InterpolationsFuzzer PROPERTIES
    COMPILE_FLAGS "${FUZZING_COMPILE_FLAGS}"
    LINK_FLAGS "${FUZZING_LINK_FLAGS}"
)
target_link_libraries(InterpolationsFuzzer ql_library ${QL_THREAD_LIBRARIES})

add_executable(TimeFunctionsFuzzer fuzztimefunctions.cpp)
set_target_properties(TimeFunctionsFuzzer PROPERTIES
    COMPILE_FLAGS "${FUZZING_COMPILE_FLAGS}"
    LINK_FLAGS "${FUZZING_LINK_FLAGS}"
)
target_link_libraries(TimeFunctionsFuzzer ql_library ${QL_THREAD_LIBRARIES})

add_executable(BlackFormulaFuzzer fuzzblackformula.cpp)
set_target_properties(BlackFormulaFuzzer PROPERTIES
    COMPILE_FLAGS "${FUZZING_COMPILE_FLAGS}"
    LINK_FLAGS "${FUZZING_LINK_FLAGS}"
)
target_link_libraries(BlackFormulaFuzzer ql_library ${QL_THREAD_LIBRARIES})