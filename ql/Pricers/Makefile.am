
# $Id$

INCLUDES= -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/Pricers

include_HEADERS = \
    americancondition.hpp \
    americanoption.hpp \
    averagepriceasian.hpp \
    averagestrikeasian.hpp \
    barrieroption.hpp \
    basket.hpp \
    bermudanoption.hpp \
    binaryoption.hpp \
    bsmnumericaloption.hpp \
    cliquetoption.hpp \
    dividendamericanoption.hpp \
    dividendeuropeanoption.hpp \
    dividendoption.hpp \
    dividendshoutoption.hpp \
    europeanengine.hpp \
    europeanoption.hpp \
    everest.hpp \
    finitedifferenceeuropean.hpp \
    geometricasianoption.hpp \
    himalaya.hpp \
    mceuropean.hpp \
    mcpricer.hpp \
    multiperiodoption.hpp \
    pagoda.hpp \
    shoutcondition.hpp \
    shoutoption.hpp \
    singleassetoption.hpp \
    stepconditionoption.hpp

EXTRA_DIST=makefile.mak

libPricers_la_SOURCES = \
    averagepriceasian.cpp \
    averagestrikeasian.cpp \
    barrieroption.cpp \
    basket.cpp \
    bermudanoption.cpp \
    binaryoption.cpp \
    bsmnumericaloption.cpp \
    cliquetoption.cpp \
    dividendamericanoption.cpp \
    dividendeuropeanoption.cpp \
    dividendoption.cpp \
    dividendshoutoption.cpp \
    europeanengine.cpp \
    europeanoption.cpp \
    everest.cpp \
    finitedifferenceeuropean.cpp \
    himalaya.cpp \
    mceuropean.cpp \
    multiperiodoption.cpp \
    pagoda.cpp \
    singleassetoption.cpp \
    stepconditionoption.cpp

noinst_LTLIBRARIES = libPricers.la
