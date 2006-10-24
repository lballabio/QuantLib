
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
    capstripper.hpp \
    cmsmarket.hpp \
    impliedvoltermstructure.hpp \
    localconstantvol.hpp \
    localvolcurve.hpp \
    localvolsurface.hpp \
    smilesection.hpp \
    swaptionconstantvol.hpp \
    swaptionvolcube.hpp \
    swaptionvolcubebylinear.hpp \
    swaptionvolcubebysabr.hpp \
    swaptionvolmatrix.hpp

libVolatilities_la_SOURCES = \
    blackvariancecurve.cpp \
    blackvariancesurface.cpp \
    capstripper.cpp \
    cmsmarket.cpp \
    localvolsurface.cpp \
    smilesection.cpp \
    swaptionvolcube.cpp \
    swaptionvolcubebylinear.cpp \
    swaptionvolcubebysabr.cpp \
    swaptionvolmatrix.cpp

noinst_LTLIBRARIES = libVolatilities.la

EXTRA_DIST = makefile.mak
