
AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
    all.hpp \
    abcdatmvolcurve.hpp \
    blackatmvolcurve.hpp \
    blackvolsurface.hpp \
    equityfxvolsurface.hpp \
    extendedblackvariancecurve.hpp \
    extendedblackvariancesurface.hpp \
    interestratevolsurface.hpp \
    noarbsabr.hpp \
    noarbsabrinterpolatedsmilesection.hpp \
    noarbsabrinterpolation.hpp \
    noarbsabrsmilesection.hpp \
    sabrvolsurface.hpp \
    sabrvoltermstructure.hpp \
    sviinterpolatedsmilesection.hpp \
    sviinterpolation.hpp \
    svismilesection.hpp \
    swaptionvolcube1a.hpp \
    volcube.hpp \
    zabr.hpp \
    zabrinterpolatedsmilesection.hpp \
    zabrinterpolation.hpp \
    zabrsmilesection.hpp

cpp_files = \
    abcdatmvolcurve.cpp \
    blackatmvolcurve.cpp \
    blackvolsurface.cpp \
    equityfxvolsurface.cpp \
    extendedblackvariancecurve.cpp \
    extendedblackvariancesurface.cpp \
    interestratevolsurface.cpp \
    noarbsabr.cpp \
    noarbsabrabsprobs.cpp \
    noarbsabrinterpolatedsmilesection.cpp \
    noarbsabrsmilesection.cpp \
    sabrvolsurface.cpp \
    sviinterpolatedsmilesection.cpp \
    svismilesection.cpp \
    volcube.cpp \
    zabr.cpp

if UNITY_BUILD

nodist_libVolatility_la_SOURCES = unity.cpp

include ${top_srcdir}/mk/unity.mk

EXTRA_DIST = $(cpp_files)

else

libVolatility_la_SOURCES = $(cpp_files)

endif

noinst_LTLIBRARIES = libVolatility.la

include ${top_srcdir}/mk/all.mk

