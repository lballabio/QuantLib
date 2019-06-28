
AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
    all.hpp \
    averageoiscouponpricer.hpp \
    arithmeticaverageois.hpp \
    arithmeticoisratehelper.hpp \
    makearithmeticaverageois.hpp

cpp_files = \
    averageoiscouponpricer.cpp \
    arithmeticaverageois.cpp \
    arithmeticoisratehelper.cpp \
    makearithmeticaverageois.cpp

if UNITY_BUILD

nodist_libAverageOIS_la_SOURCES = unity.cpp

include ${top_srcdir}/mk/unity.mk

EXTRA_DIST = $(cpp_files)

else

libAverageOIS_la_SOURCES = $(cpp_files)

endif

noinst_LTLIBRARIES = libAverageOIS.la

include ${top_srcdir}/mk/all.mk

