
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/MarketModels

SUBDIRS = Products PseudoRoots BrownianGenerators Evolvers

include_HEADERS = \
    all.hpp \
    core.hpp \
    accountingengine.hpp \
    browniangenerator.hpp \
    curvestate.hpp \
    driftcalculator.hpp \
    evolutiondescription.hpp \
    marketmodelevolver.hpp \
    marketmodelproduct.hpp \
    pseudoroot.hpp

libMarketModels_la_SOURCES = \
    accountingengine.cpp \
    curvestate.cpp \
    driftcalculator.cpp \
    evolutiondescription.cpp


noinst_LTLIBRARIES = libMarketModels.la

libMarketModels_la_LIBADD = \
    BrownianGenerators/libMarketModelsBrownianGenerators.la \
    Evolvers/libMarketModelsEvolvers.la \
    Products/libMarketModelsProducts.la \
    PseudoRoots/libMarketModelsPseudoRoots.la

#EXTRA_DIST = makefile.mak

