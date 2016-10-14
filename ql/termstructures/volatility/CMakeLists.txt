install(FILES
    all.hpp
    abcd.hpp
    abcdcalibration.hpp
    atmadjustedsmilesection.hpp
    atmsmilesection.hpp
    flatsmilesection.hpp
    gaussian1dsmilesection.hpp
    interpolatedsmilesection.hpp
    kahalesmilesection.hpp
    sabr.hpp
    sabrinterpolatedsmilesection.hpp
    sabrsmilesection.hpp
    smilesection.hpp
    smilesectionutils.hpp
    spreadedsmilesection.hpp
    volatilitytype.hpp
    DESTINATION include/ql/termstructures/volatility)

add_subdirectory(capfloor)
add_subdirectory(equityfx)
add_subdirectory(inflation)
add_subdirectory(optionlet)
add_subdirectory(swaption)
