
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/Volatilities

include_HEADERS = \
    all.hpp \
    blackconstantvol.hpp \
    blackvariancecurve.hpp \
    blackvariancesurface.hpp \
    capflatvolvector.hpp \
    capletconstantvol.hpp \
    impliedvoltermstructure.hpp \
    localconstantvol.hpp \
    localvolcurve.hpp \
    localvolsurface.hpp \
    swaptionvolmatrix.hpp

EXTRA_DIST=makefile.mak

libVolatilities_la_SOURCES = \
    blackvariancecurve.cpp \
    blackvariancesurface.cpp \
    localvolsurface.cpp

noinst_LTLIBRARIES = libVolatilities.la
