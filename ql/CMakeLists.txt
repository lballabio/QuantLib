file(GLOB_RECURSE QUANTLIB_FILES "*.hpp" "*.cpp")
add_library(QuantLibObjects OBJECT ${QUANTLIB_FILES})
add_library(QuantLibStatic STATIC $<TARGET_OBJECTS:QuantLibObjects>)
set_target_properties(QuantLibStatic PROPERTIES OUTPUT_NAME QuantLib)
install(TARGETS QuantLibStatic ARCHIVE DESTINATION lib)
if (NOT DISABLE_SHARED)
    add_library(QuantLib SHARED $<TARGET_OBJECTS:QuantLibObjects>)
    install(TARGETS QuantLib LIBRARY DESTINATION lib)
endif (NOT DISABLE_SHARED)

install(FILES 
    auto_link.hpp
    cashflow.hpp
    compounding.hpp
    currency.hpp
    default.hpp
    discretizedasset.hpp
    errors.hpp
    exchangerate.hpp
    exercise.hpp
    event.hpp
    grid.hpp
    handle.hpp
    index.hpp
    instrument.hpp
    interestrate.hpp
    mathconstants.hpp
    money.hpp
    numericalmethod.hpp
    option.hpp
    payoff.hpp
    position.hpp
    prices.hpp
    pricingengine.hpp
    qldefines.hpp
    quantlib.hpp
    quote.hpp
    rebatedexercise.hpp
    settings.hpp
    stochasticprocess.hpp
    termstructure.hpp
    timegrid.hpp
    timeseries.hpp
    types.hpp
    version.hpp
    volatilitymodel.hpp
    DESTINATION include/ql)

install(FILES
    config.ansi.hpp
    config.msvc.hpp
    config.mingw.hpp
    config.sun.hpp
    userconfig.hpp
    DESTINATION include/ql)

add_subdirectory(cashflows)
add_subdirectory(currencies)
add_subdirectory(experimental)
add_subdirectory(indexes)
add_subdirectory(instruments)
add_subdirectory(legacy)
add_subdirectory(math)
add_subdirectory(methods)
add_subdirectory(models)
add_subdirectory(patterns)
add_subdirectory(pricingengines)
add_subdirectory(processes)
add_subdirectory(quotes)
add_subdirectory(termstructures)
add_subdirectory(time)
add_subdirectory(utilities)
