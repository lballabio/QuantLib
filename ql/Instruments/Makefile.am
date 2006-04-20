
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/Instruments

include_HEADERS = \
    all.hpp \
    core.hpp \
    asianoption.hpp \
    barrieroption.hpp \
    basketoption.hpp \
    bond.hpp \
    callabilityschedule.hpp \
    capfloor.hpp \
    cliquetoption.hpp \
    convertiblebond.hpp \
    dividendvanillaoption.hpp \
    dividendschedule.hpp \
    europeanoption.hpp \
    fixedcouponbond.hpp \
    floatingratebond.hpp \
    forwardvanillaoption.hpp \
    lookbackoption.hpp \
    multiassetoption.hpp \
    oneassetoption.hpp \
    oneassetstrikedoption.hpp \
    payoffs.hpp \
    quantoforwardvanillaoption.hpp \
    quantovanillaoption.hpp \
    simpleswap.hpp \
    stock.hpp \
    swap.hpp \
    swaption.hpp \
    vanillaoption.hpp \
    zerocouponbond.hpp

libInstruments_la_SOURCES = \
    asianoption.cpp \
    barrieroption.cpp \
    basketoption.cpp \
    bond.cpp \
    capfloor.cpp \
    cliquetoption.cpp \
    convertiblebond.cpp \
    dividendvanillaoption.cpp \
    europeanoption.cpp \
    fixedcouponbond.cpp \
    floatingratebond.cpp \
    forwardvanillaoption.cpp \
    lookbackoption.cpp \
    multiassetoption.cpp \
    oneassetoption.cpp \
    oneassetstrikedoption.cpp \
    quantoforwardvanillaoption.cpp \
    quantovanillaoption.cpp \
    simpleswap.cpp \
    stock.cpp \
    swap.cpp \
    swaption.cpp \
    vanillaoption.cpp \
    zerocouponbond.cpp

noinst_LTLIBRARIES = libInstruments.la

EXTRA_DIST = makefile.mak

