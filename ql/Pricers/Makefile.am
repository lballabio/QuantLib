
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/Pricers

include_HEADERS = \
    all.hpp \
    core.hpp \
    cliquetoptionpricer.hpp \
    continuousgeometricapo.hpp \
    discretegeometricapo.hpp \
    discretegeometricaso.hpp \
    dividendeuropeanoption.hpp \
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
    mcbasket.hpp \
    mccliquetoption.hpp \
    mcdiscretearithmeticapo.hpp \
    mcdiscretearithmeticaso.hpp \
    mceverest.hpp \
    mchimalaya.hpp \
    mcmaxbasket.hpp \
    mcpagoda.hpp \
    mcperformanceoption.hpp \
    mcpricer.hpp \
    performanceoption.hpp \
    singleassetoption.hpp

EXTRA_DIST=makefile.mak

libPricers_la_SOURCES = \
    cliquetoptionpricer.cpp \
    discretegeometricapo.cpp \
    discretegeometricaso.cpp \
    dividendeuropeanoption.cpp \
    fdbermudanoption.cpp \
    fdbsmoption.cpp \
    fddividendamericanoption.cpp \
    fddividendoption.cpp \
    fddividendshoutoption.cpp \
    fdeuropean.cpp \
    fdmultiperiodoption.cpp \
    fdstepconditionoption.cpp \
    mcbasket.cpp \
    mccliquetoption.cpp \
    mcdiscretearithmeticapo.cpp \
    mcdiscretearithmeticaso.cpp \
    mceverest.cpp \
    mchimalaya.cpp \
    mcmaxbasket.cpp \
    mcpagoda.cpp \
    mcperformanceoption.cpp \
    performanceoption.cpp \
    singleassetoption.cpp

noinst_LTLIBRARIES = libPricers.la
