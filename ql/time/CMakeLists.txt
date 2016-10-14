install(FILES
    all.hpp
    asx.hpp
    businessdayconvention.hpp
    calendar.hpp
    date.hpp
    dategenerationrule.hpp
    daycounter.hpp
    ecb.hpp
    frequency.hpp
    imm.hpp
    period.hpp
    schedule.hpp
    timeunit.hpp
    weekday.hpp
    DESTINATION include/ql/time)

add_subdirectory(calendars)
add_subdirectory(daycounters)
