
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/TermStructures

include_HEADERS = \
    affinetermstructure.hpp \
    all.hpp \
    bootstraptraits.hpp \
    compoundforward.hpp \
    discountcurve.hpp \
    drifttermstructure.hpp \
    extendeddiscountcurve.hpp \
    flatforward.hpp \
    forwardspreadedtermstructure.hpp \
    forwardstructure.hpp \
    impliedtermstructure.hpp \
    piecewiseflatforward.hpp \
    piecewiseyieldcurve.hpp \
    quantotermstructure.hpp \
    ratehelpers.hpp \
    zerocurve.hpp \
    zerospreadedtermstructure.hpp \
    zeroyieldstructure.hpp

EXTRA_DIST=makefile.mak

libTermStructures_la_SOURCES = \
    affinetermstructure.cpp \
    compoundforward.cpp \
    extendeddiscountcurve.cpp \
    piecewiseflatforward.cpp \
    ratehelpers.cpp

noinst_LTLIBRARIES = libTermStructures.la
