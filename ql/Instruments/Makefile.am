
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/Instruments

include_HEADERS = \
    all.hpp \
    core.hpp \
    asianoption.hpp \
    barrieroption.hpp \
    basketoption.hpp \
    bond.hpp \
    capfloor.hpp \
    cliquetoption.hpp \
    dividendvanillaoption.hpp \
    dividendschedule.hpp \
    europeanoption.hpp \
    fixedcouponbond.hpp \
    floatingratebond.hpp \
    forwardvanillaoption.hpp \
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

EXTRA_DIST=makefile.mak

libInstruments_la_SOURCES = \
    asianoption.cpp \
    barrieroption.cpp \
    basketoption.cpp \
    bond.cpp \
    capfloor.cpp \
    cliquetoption.cpp \
    dividendvanillaoption.cpp \
    europeanoption.cpp \
    fixedcouponbond.cpp \
    floatingratebond.cpp \
    forwardvanillaoption.cpp \
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
