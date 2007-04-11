
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
    all.hpp \
    bondhelpers.hpp \
    bootstraptraits.hpp \
    compoundforward.hpp \
    discountcurve.hpp \
    drifttermstructure.hpp \
    extendeddiscountcurve.hpp \
    flatforward.hpp \
    forwardcurve.hpp \
    forwardspreadedtermstructure.hpp \
    forwardstructure.hpp \
    impliedtermstructure.hpp \
    piecewiseyieldcurve.hpp \
    piecewisezerospreadedtermstructure.hpp \
    quantotermstructure.hpp \
    ratehelpers.hpp \
    zerocurve.hpp \
    zerospreadedtermstructure.hpp \
    zeroyieldstructure.hpp

libYieldCurves_la_SOURCES = \
    bondhelpers.cpp \
    compoundforward.cpp \
    extendeddiscountcurve.cpp \
    piecewiseyieldcurve.cpp \
    ratehelpers.cpp

noinst_LTLIBRARIES = libYieldCurves.la
