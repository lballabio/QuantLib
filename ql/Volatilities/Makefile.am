
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/Volatilities

include_HEADERS = \
    all.hpp \
    blackconstantvol.hpp \
    blackvariancecurve.hpp \
    blackvariancesurface.hpp \
    capflatvolvector.hpp \
    capletconstantvol.hpp \
    capletvariancecurve.hpp \
    capletvolatilitiesstructures.hpp \
    capstripper.hpp \
    cmsmarket.hpp \
    impliedvoltermstructure.hpp \
    localconstantvol.hpp \
    localvolcurve.hpp \
    localvolsurface.hpp \
    sabr.hpp \
    smilesection.hpp \
    swaptionconstantvol.hpp \
    swaptionvolcube.hpp \
    swaptionvolcube1.hpp \
    swaptionvolcube2.hpp \
    swaptionvoldiscrete.hpp \
    swaptionvolmatrix.hpp

libVolatilities_la_SOURCES = \
    blackvariancecurve.cpp \
    blackvariancesurface.cpp \
    capletvolatilitiesstructures.cpp \
    capstripper.cpp \
    cmsmarket.cpp \
    localvolsurface.cpp \
    sabr.cpp \
    smilesection.cpp \
    swaptionvolcube.cpp \
    swaptionvolcube1.cpp \
    swaptionvolcube2.cpp \
    swaptionvoldiscrete.cpp \
    swaptionvolmatrix.cpp

noinst_LTLIBRARIES = libVolatilities.la

EXTRA_DIST = makefile.mak
