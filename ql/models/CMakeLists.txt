install(FILES
    all.hpp
    calibrationhelper.hpp
    model.hpp
    parameter.hpp
    DESTINATION include/ql/models)

add_subdirectory(equity)
add_subdirectory(marketmodels)
add_subdirectory(shortrate)
add_subdirectory(volatility)
