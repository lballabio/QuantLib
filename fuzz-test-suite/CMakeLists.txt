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

add_executable(VanillaOptionFuzzer fuzz_vanilla_option.cpp)
set_target_properties(VanillaOptionFuzzer PROPERTIES
    COMPILE_FLAGS "${FUZZING_COMPILE_FLAGS}"
    LINK_FLAGS "${FUZZING_LINK_FLAGS}"
)
target_link_libraries(VanillaOptionFuzzer ql_library ${QL_THREAD_LIBRARIES})

add_executable(Fuzz_CashFlows fuzz_cashflows.cpp)
set_target_properties(Fuzz_CashFlows PROPERTIES
    COMPILE_FLAGS "${FUZZING_COMPILE_FLAGS}"
    LINK_FLAGS "${FUZZING_LINK_FLAGS}"
)
target_link_libraries(Fuzz_CashFlows ql_library ${QL_THREAD_LIBRARIES})

add_executable(Fuzz_IndexManager fuzz_indexmanager.cpp)
set_target_properties(Fuzz_IndexManager PROPERTIES
    COMPILE_FLAGS "${FUZZING_COMPILE_FLAGS}"
    LINK_FLAGS "${FUZZING_LINK_FLAGS}"
)
target_link_libraries(Fuzz_IndexManager ql_library ${QL_THREAD_LIBRARIES})

add_executable(Fuzz_ExchangeRateManager fuzz_exchangeratemanager.cpp)
set_target_properties(Fuzz_ExchangeRateManager PROPERTIES
    COMPILE_FLAGS "${FUZZING_COMPILE_FLAGS}"
    LINK_FLAGS "${FUZZING_LINK_FLAGS}"
)
target_link_libraries(Fuzz_ExchangeRateManager ql_library ${QL_THREAD_LIBRARIES})

add_executable(Fuzz_BarrierOption fuzz_barrieroption.cpp)
set_target_properties(Fuzz_BarrierOption PROPERTIES
    COMPILE_FLAGS "${FUZZING_COMPILE_FLAGS}"
    LINK_FLAGS "${FUZZING_LINK_FLAGS}"
)
target_link_libraries(Fuzz_BarrierOption ql_library ${QL_THREAD_LIBRARIES})

add_executable(Fuzz_AsianOption fuzz_asianoption.cpp)
set_target_properties(Fuzz_AsianOption PROPERTIES
    COMPILE_FLAGS "${FUZZING_COMPILE_FLAGS}"
    LINK_FLAGS "${FUZZING_LINK_FLAGS}"
)
target_link_libraries(Fuzz_AsianOption ql_library ${QL_THREAD_LIBRARIES})

add_executable(Fuzz_BasketOption fuzz_basketoption.cpp)
set_target_properties(Fuzz_BasketOption PROPERTIES
    COMPILE_FLAGS "${FUZZING_COMPILE_FLAGS}"
    LINK_FLAGS "${FUZZING_LINK_FLAGS}"
)
target_link_libraries(Fuzz_BasketOption ql_library ${QL_THREAD_LIBRARIES})