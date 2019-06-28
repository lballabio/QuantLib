
AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
    all.hpp \
    inflationhelpers.hpp \
    inflationtraits.hpp \
    interpolatedyoyinflationcurve.hpp \
    interpolatedzeroinflationcurve.hpp \
    piecewiseyoyinflationcurve.hpp \
    piecewisezeroinflationcurve.hpp \
    seasonality.hpp

cpp_files = \
    inflationhelpers.cpp \
    seasonality.cpp

if UNITY_BUILD

nodist_libInflationTermStructures_la_SOURCES = unity.cpp

include ${top_srcdir}/mk/unity.mk

EXTRA_DIST = $(cpp_files)

else

libInflationTermStructures_la_SOURCES = $(cpp_files)

endif

noinst_LTLIBRARIES = libInflationTermStructures.la

include ${top_srcdir}/mk/all.mk

