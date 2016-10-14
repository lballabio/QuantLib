set(INCLUDE_HEADERS
    all.hpp
    bootstraperror.hpp
    bootstraphelper.hpp
    defaulttermstructure.hpp
    inflationtermstructure.hpp
    interpolatedcurve.hpp
    iterativebootstrap.hpp
    localbootstrap.hpp
    voltermstructure.hpp
    yieldtermstructure.hpp)

install(FILES ${INCLUDE_HEADERS} DESTINATION include/ql/termstructures)

add_subdirectory(credit)
add_subdirectory(inflation)
add_subdirectory(volatility)
add_subdirectory(yield)
