# defines options to define the preprocessors in <ql/userconfig.hpp>

macro(addBoostThreadLibrary)
    find_package(Boost REQUIRED COMPONENTS thread)
    include_directories(${Boost_INCLUDE_DIRS})
    list(APPEND EXTRA_LIBS ${Boost_LIBRARIES})
endmacro(addBoostThreadLibrary)

macro(setPreprocessor PREPROCESSOR_NAME VALUE)
    if(${VALUE})
        add_definitions(-D${PREPROCESSOR_NAME})
    endif()
endmacro()

set(PREPROCESSOR_COMMENT "Enables the correspondig preprocessors in <ql/userconfig.hpp>.")

list(APPEND CONFIG_PREPROCESSORS
    QL_ERROR_FUNCTIONS
    QL_ERROR_LINES
    QL_ENABLE_TRACING
    QL_EXTRA_SAFETY_CHECKS
    QL_USE_INDEXED_COUPON
    QL_ENABLE_SESSIONS
    QL_ENABLE_THREAD_SAFE_OBSERVER_PATTERN
    QL_HIGH_RESOLUTION_DATE
    QL_USE_STD_SHARED_PTR
    # QL_USE_STD_UNIQUE_PTR                     ### skipped as it cannot be switch off without skipping <ql/userconfig.hpp>
    QL_USE_STD_FUNCTION
    QL_USE_STD_TUPLE
    QL_USE_DISPOSABLE
    QL_ENABLE_PARALLEL_UNIT_TEST_RUNNER
    QL_ENABLE_SINGLETON_THREAD_SAFE_INIT
)

foreach(item ${CONFIG_PREPROCESSORS})
    option(         ${item}     ${PREPROCESSOR_COMMENT})
    setPreprocessor(${item}     ${${item}})
endforeach()

if(${QL_ENABLE_SESSIONS} OR ${QL_ENABLE_THREAD_SAFE_OBSERVER_PATTERN} OR ${QL_ENABLE_SINGLETON_THREAD_SAFE_INIT})
    addBoostThreadLibrary()
endif()
