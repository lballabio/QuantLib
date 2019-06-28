
AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
    all.hpp \
    bondhelpers.hpp \
    bootstraptraits.hpp \
    compositezeroyieldstructure.hpp \
    discountcurve.hpp \
    drifttermstructure.hpp \
    fittedbonddiscountcurve.hpp \
    flatforward.hpp \
    forwardcurve.hpp \
    forwardspreadedtermstructure.hpp \
    forwardstructure.hpp \
    impliedtermstructure.hpp \
    nonlinearfittingmethods.hpp \
    oisratehelper.hpp \
    piecewiseyieldcurve.hpp \
    piecewisezerospreadedtermstructure.hpp \
    quantotermstructure.hpp \
    ratehelpers.hpp \
    zerocurve.hpp \
    zerospreadedtermstructure.hpp \
    zeroyieldstructure.hpp

cpp_files = \
    bondhelpers.cpp \
    fittedbonddiscountcurve.cpp \
    flatforward.cpp \
    forwardstructure.cpp \
    nonlinearfittingmethods.cpp \
    oisratehelper.cpp \
    ratehelpers.cpp \
    zeroyieldstructure.cpp

if UNITY_BUILD

nodist_libYieldTermStructures_la_SOURCES = unity.cpp

include ${top_srcdir}/mk/unity.mk

EXTRA_DIST = $(cpp_files)

else

libYieldTermStructures_la_SOURCES = $(cpp_files)

endif

noinst_LTLIBRARIES = libYieldTermStructures.la

include ${top_srcdir}/mk/all.mk

