
AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
    all.hpp \
    defaultdensitystructure.hpp \
    defaultprobabilityhelpers.hpp \
    flathazardrate.hpp \
    hazardratestructure.hpp \
    interpolateddefaultdensitycurve.hpp \
    interpolatedhazardratecurve.hpp \
    interpolatedsurvivalprobabilitycurve.hpp \
    piecewisedefaultcurve.hpp \
    probabilitytraits.hpp \
    survivalprobabilitystructure.hpp

cpp_files = \
    defaultdensitystructure.cpp \
    defaultprobabilityhelpers.cpp \
    flathazardrate.cpp \
    hazardratestructure.cpp \
    survivalprobabilitystructure.cpp

if UNITY_BUILD

nodist_libDefaultTermStructures_la_SOURCES = unity.cpp

include ${top_srcdir}/mk/unity.mk

EXTRA_DIST = $(cpp_files)

else

libDefaultTermStructures_la_SOURCES = $(cpp_files)

endif

noinst_LTLIBRARIES = libDefaultTermStructures.la

include ${top_srcdir}/mk/all.mk

