
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/Pricers

include_HEADERS = \
    all.hpp \
    core.hpp \
    discretegeometricaso.hpp \
    fdamericanoption.hpp \
    fdbermudanoption.hpp \
    fdbsmoption.hpp \
    fddividendamericanoption.hpp \
    fddividendoption.hpp \
    fddividendshoutoption.hpp \
    fdeuropean.hpp \
    fdmultiperiodoption.hpp \
    fdshoutoption.hpp \
    fdstepconditionoption.hpp \
    mccliquetoption.hpp \
    mcdiscretearithmeticaso.hpp \
    mceverest.hpp \
    mchimalaya.hpp \
    mcmaxbasket.hpp \
    mcpagoda.hpp \
    mcperformanceoption.hpp \
    mcpricer.hpp \
    singleassetoption.hpp

EXTRA_DIST=makefile.mak

libPricers_la_SOURCES = \
    discretegeometricaso.cpp \
    fdbermudanoption.cpp \
    fdbsmoption.cpp \
    fddividendamericanoption.cpp \
    fddividendoption.cpp \
    fddividendshoutoption.cpp \
    fdeuropean.cpp \
    fdmultiperiodoption.cpp \
    fdstepconditionoption.cpp \
    mccliquetoption.cpp \
    mcdiscretearithmeticaso.cpp \
    mceverest.cpp \
    mchimalaya.cpp \
    mcmaxbasket.cpp \
    mcpagoda.cpp \
    mcperformanceoption.cpp \
    singleassetoption.cpp

noinst_LTLIBRARIES = libPricers.la
