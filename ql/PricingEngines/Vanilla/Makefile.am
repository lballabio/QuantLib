
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/PricingEngines/Vanilla

include_HEADERS = \
    all.hpp \
    analyticdigitalamericanengine.hpp \
    analyticdividendeuropeanengine.hpp \
    analyticeuropeanengine.hpp \
    baroneadesiwhaleyengine.hpp \
    binomialengine.hpp \
    bjerksundstenslandengine.hpp \
    discretizedvanillaoption.hpp \
    integralengine.hpp \
    jumpdiffusionengine.hpp \
    juquadraticengine.hpp \
    mcdigitalengine.hpp \
    mceuropeanengine.hpp \
    mcvanillaengine.hpp

EXTRA_DIST = makefile.mak

libVanillaEngines_la_SOURCES = \
    analyticdigitalamericanengine.cpp \
    analyticdividendeuropeanengine.cpp \
    analyticeuropeanengine.cpp \
    baroneadesiwhaleyengine.cpp \
    bjerksundstenslandengine.cpp \
    discretizedvanillaoption.cpp \
    integralengine.cpp \
    jumpdiffusionengine.cpp \
    juquadraticengine.cpp \
    mcdigitalengine.cpp

noinst_LTLIBRARIES = libVanillaEngines.la

