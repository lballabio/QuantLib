
# $Id$

INCLUDES= -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/Pricers

include_HEADERS = \
    barrieroption.hpp \
    binaryoption.hpp \
    cliquetoption.hpp \
    continuousgeometricapo.hpp \
    couponbondoption.hpp \
    discretegeometricapo.hpp \
    discretegeometricaso.hpp \
    europeanengine.hpp \
    europeanoption.hpp \
    fdamericanoption.hpp \
    fdbermudanoption.hpp \
    fdbsmoption.hpp \
    fddividendamericanoption.hpp \
    fddividendeuropeanoption.hpp \
    fddividendoption.hpp \
    fddividendshoutoption.hpp \
    fdeuropean.hpp \
    fdeuropeanswaption.hpp \
    fdmultiperiodoption.hpp \
    fdshoutoption.hpp \
    fdstepconditionoption.hpp \
    mcbasket.hpp \
    mcdiscretearithmeticapo.hpp \
    mcdiscretearithmeticaso.hpp \
    mceuropean.hpp \
    mceverest.hpp \
    mchimalaya.hpp \
    mcmaxbasket.hpp \
    mcpagoda.hpp \
    mcpricer.hpp \
    singleassetoption.hpp

EXTRA_DIST=makefile.mak

libPricers_la_SOURCES = \
    barrieroption.cpp \
    fdbermudanoption.cpp \
    binaryoption.cpp \
    cliquetoption.cpp \
    couponbondoption.cpp \
    discretegeometricapo.cpp \
    discretegeometricaso.cpp \
    fddividendshoutoption.cpp \
    europeanengine.cpp \
    europeanoption.cpp \
    fdbsmoption.cpp \
    fddividendamericanoption.cpp \
    fddividendeuropeanoption.cpp \
    fddividendoption.cpp \
    fdeuropean.cpp \
    fdeuropeanswaption.cpp \
    fdmultiperiodoption.cpp \
    fdstepconditionoption.cpp \
    mcbasket.cpp \
    mcdiscretearithmeticapo.cpp \
    mcdiscretearithmeticaso.cpp \
    mceuropean.cpp \
    mceverest.cpp \
    mchimalaya.cpp \
    mcmaxbasket.cpp \
    mcpagoda.cpp \
    singleassetoption.cpp

noinst_LTLIBRARIES = libPricers.la
