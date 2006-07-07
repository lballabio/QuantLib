
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/MarketModels

SUBDIRS = Products

include_HEADERS = \
    all.hpp \
    core.hpp \
    abcdvolatility.hpp \
    accountingengine.hpp \
    browniangenerator.hpp \
    curvestate.hpp \
    driftcalculator.hpp \
    evolutiondescription.hpp \
    exponentialcorrelation.hpp \
    forwardrateevolver.hpp \
    forwardrateipcevolver.hpp \
    marketmodelevolver.hpp \
    marketmodelproduct.hpp \
    mtbrowniangenerator.hpp \
    pseudoroot.hpp

libMarketModels_la_SOURCES = \
    abcdvolatility.cpp \
    accountingengine.cpp \
    curvestate.cpp \
    driftcalculator.cpp \
    evolutiondescription.cpp \
    exponentialcorrelation.cpp \
    forwardrateevolver.cpp \
    forwardrateipcevolver.cpp \
    mtbrowniangenerator.cpp

noinst_LTLIBRARIES = libMarketModels.la

libMarketModels_la_LIBADD = \
    Products/libMarketModelsProducts.la

#EXTRA_DIST = makefile.mak

