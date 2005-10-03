
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/TermStructures

include_HEADERS = \
    affinetermstructure.hpp \
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
    piecewiseflatforward.hpp \
    piecewiseyieldcurve.hpp \
    quantotermstructure.hpp \
    ratehelpers.hpp \
    zerocurve.hpp \
    zerospreadedtermstructure.hpp \
    zeroyieldstructure.hpp

libTermStructures_la_SOURCES = \
    affinetermstructure.cpp \
    bondhelpers.cpp \
    compoundforward.cpp \
    extendeddiscountcurve.cpp \
    piecewiseflatforward.cpp \
    ratehelpers.cpp

noinst_LTLIBRARIES = libTermStructures.la

EXTRA_DIST = makefile.mak

