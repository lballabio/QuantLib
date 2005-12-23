
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/PricingEngines/Vanilla

include_HEADERS = \
    all.hpp \
    analyticdigitalamericanengine.hpp \
    analyticdividendeuropeanengine.hpp \
    analyticeuropeanengine.hpp \
    analytichestonengine.hpp \
    baroneadesiwhaleyengine.hpp \
    batesengine.hpp \
    binomialengine.hpp \
    bjerksundstenslandengine.hpp \
    discretizedvanillaoption.hpp \
    integralengine.hpp \
    jumpdiffusionengine.hpp \
    juquadraticengine.hpp \
    fdamericanengine.hpp \
    fdbermudanengine.hpp \
    fddividendamericanengine.hpp \
    fddividendengine.hpp \
    fddividendeuropeanengine.hpp \
    fddividendshoutengine.hpp \
    fdeuropeanengine.hpp \
    fdmultiperiodengine.hpp \
    fdshoutengine.hpp \
    fdstepconditionengine.hpp \
    fdvanillaengine.hpp \
    fdconditions.hpp \
    mcdigitalengine.hpp \
    mceuropeanengine.hpp \
    mceuropeanhestonengine.hpp \
    mcvanillaengine.hpp

libVanillaEngines_la_SOURCES = \
    analyticdigitalamericanengine.cpp \
    analyticdividendeuropeanengine.cpp \
    analyticeuropeanengine.cpp \
    analytichestonengine.cpp \
    baroneadesiwhaleyengine.cpp \
    batesengine.cpp \
    bjerksundstenslandengine.cpp \
    discretizedvanillaoption.cpp \
    integralengine.cpp \
    jumpdiffusionengine.cpp \
    juquadraticengine.cpp \
    fddividendengine.cpp \
    fdeuropeanengine.cpp \
    fdmultiperiodengine.cpp \
    fdstepconditionengine.cpp \
    fdvanillaengine.cpp \
    mcdigitalengine.cpp

noinst_LTLIBRARIES = libVanillaEngines.la

EXTRA_DIST = makefile.mak

