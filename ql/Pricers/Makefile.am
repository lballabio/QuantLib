
# $Id$

INCLUDES= -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/Pricers

include_HEADERS = \
    americancondition.hpp \
    americanoption.hpp \
    barrieroption.hpp \
    bermudanoption.hpp \
    binaryoption.hpp \
    bsmnumericaloption.hpp \
    cliquetoption.hpp \
    continuousgeometricapo.hpp \
    discretegeometricaso.hpp \
    discretegeometricapo.hpp \
    dividendamericanoption.hpp \
    dividendeuropeanoption.hpp \
    dividendoption.hpp \
    dividendshoutoption.hpp \
    europeanengine.hpp \
    europeanoption.hpp \
    finitedifferenceeuropean.hpp \
    mcbasket.hpp \
    mcdiscretearithmeticapo.hpp \
    mcdiscretearithmeticaso.hpp \
    mceuropean.hpp \
    mceverest.hpp \
    mchimalaya.hpp \
    mcpagoda.hpp \
    mcpricer.hpp \
    multiperiodoption.hpp \
    shoutcondition.hpp \
    shoutoption.hpp \
    singleassetoption.hpp \
    stepconditionoption.hpp

EXTRA_DIST=makefile.mak

libPricers_la_SOURCES = \
    barrieroption.cpp \
    bermudanoption.cpp \
    binaryoption.cpp \
    bsmnumericaloption.cpp \
    cliquetoption.cpp \
    discretegeometricaso.cpp \
    discretegeometricapo.cpp \
    dividendamericanoption.cpp \
    dividendeuropeanoption.cpp \
    dividendoption.cpp \
    dividendshoutoption.cpp \
    europeanengine.cpp \
    europeanoption.cpp \
    finitedifferenceeuropean.cpp \
    mcbasket.cpp \
    mcdiscretearithmeticapo.cpp \
    mcdiscretearithmeticaso.cpp \
    mceuropean.cpp \
    mceverest.cpp \
    mchimalaya.cpp \
    mcpagoda.cpp \
    multiperiodoption.cpp \
    singleassetoption.cpp \
    stepconditionoption.cpp

noinst_LTLIBRARIES = libPricers.la
