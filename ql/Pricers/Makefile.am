
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/Pricers

include_HEADERS = \
    all.hpp \
    core.hpp \
    discretegeometricaso.hpp \
    mccliquetoption.hpp \
    mcdiscretearithmeticaso.hpp \
    mceverest.hpp \
    mchimalaya.hpp \
    mcmaxbasket.hpp \
    mcpagoda.hpp \
    mcperformanceoption.hpp \
    mcpricer.hpp \
    singleassetoption.hpp

libPricers_la_SOURCES = \
    discretegeometricaso.cpp \
    mccliquetoption.cpp \
    mcdiscretearithmeticaso.cpp \
    mceverest.cpp \
    mchimalaya.cpp \
    mcmaxbasket.cpp \
    mcpagoda.cpp \
    mcperformanceoption.cpp \
    singleassetoption.cpp

noinst_LTLIBRARIES = libPricers.la

EXTRA_DIST = makefile.mak

