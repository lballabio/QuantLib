
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/termstructures/yieldcurves

include_HEADERS = \
    all.hpp \
    compoundforward.hpp \
    discountcurve.hpp \
    extendeddiscountcurve.hpp \
    flatforward.hpp \
    forwardcurve.hpp \
    forwardspreadedtermstructure.hpp \
    forwardstructure.hpp \
    impliedtermstructure.hpp \
    piecewiseflatforward.hpp \
    piecewiseyieldcurve.hpp \
    piecewisezerospreadedtermstructure.hpp \
    zerocurve.hpp \
    zerospreadedtermstructure.hpp \
    zeroyieldstructure.hpp

libYieldCurves_la_SOURCES = \
    compoundforward.cpp \
    extendeddiscountcurve.cpp \
    piecewiseflatforward.cpp \
    piecewiseyieldcurve.cpp

noinst_LTLIBRARIES = libYieldCurves.la
