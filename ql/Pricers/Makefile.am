
# $Id$

INCLUDES= -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/Pricers

include_HEADERS = \
    americancondition.hpp \
    americanoption.hpp \
    averagepriceasian.hpp \
    averagestrikeasian.hpp \
    barrieroption.hpp \
    bermudanoption.hpp \
    binaryoption.hpp \
    bsmnumericaloption.hpp \
    cliquetoption.hpp \
    dividendamericanoption.hpp \
    dividendeuropeanoption.hpp \
    dividendoption.hpp \
    dividendshoutoption.hpp \
    europeanoption.hpp \
    everestoption.hpp \
    finitedifferenceeuropean.hpp \
    geometricasianoption.hpp \
    himalaya.hpp \
    mceuropeanpricer.hpp \
    multiperiodoption.hpp \
    pagodaoption.hpp \
    plainbasketoption.hpp \
    shoutcondition.hpp \
    shoutoption.hpp \
    singleassetoption.hpp \
    stepconditionoption.hpp

EXTRA_DIST=makefile.mak

libPricers_la_SOURCES = \
    averagepriceasian.cpp \
    averagestrikeasian.cpp \
    barrieroption.cpp \
    binaryoption.cpp \
    bermudanoption.cpp \
    bsmnumericaloption.cpp \
    cliquetoption.cpp \
    dividendoption.cpp \
    dividendshoutoption.cpp \
    dividendamericanoption.cpp \
    dividendeuropeanoption.cpp \
    europeanoption.cpp \
    everestoption.cpp \
    finitedifferenceeuropean.cpp \
    himalaya.cpp \
    mceuropeanpricer.cpp \
    multiperiodoption.cpp \
    plainbasketoption.cpp \
    pagodaoption.cpp \
    singleassetoption.cpp \
    stepconditionoption.cpp

noinst_LTLIBRARIES = libPricers.la
