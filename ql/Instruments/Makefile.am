
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/Instruments

include_HEADERS = \
    all.hpp \
    asianoption.hpp \
    barrieroption.hpp \
    basketoption.hpp \
    bond.hpp \
    callabilityschedule.hpp \
    capfloor.hpp \
    cliquetoption.hpp \
    compositeinstrument.hpp \
    convertiblebond.hpp \
    core.hpp \
    dividendschedule.hpp \
    dividendvanillaoption.hpp \
    europeanoption.hpp \
    fixedcouponbond.hpp \
    fixedcouponbondforward.hpp \
    floatingratebond.hpp \
    forward.hpp \
    forwardrateagreement.hpp \
    forwardvanillaoption.hpp \
    lookbackoption.hpp \
    multiassetoption.hpp \
    oneassetoption.hpp \
    oneassetstrikedoption.hpp \
    payoffs.hpp \
    quantoforwardvanillaoption.hpp \
    quantovanillaoption.hpp \
    stock.hpp \
    swap.hpp \
    swaption.hpp \
    vanillaoption.hpp \
    vanillaswap.hpp \
    varianceswap.hpp \
    zerocouponbond.hpp

libInstruments_la_SOURCES = \
    asianoption.cpp \
    barrieroption.cpp \
    basketoption.cpp \
    bond.cpp \
    capfloor.cpp \
    cliquetoption.cpp \
    compositeinstrument.cpp \
    convertiblebond.cpp \
    dividendvanillaoption.cpp \
    europeanoption.cpp \
    fixedcouponbond.cpp \
    fixedcouponbondforward.cpp \
    floatingratebond.cpp \
    forward.cpp \
    forwardrateagreement.cpp \
    forwardvanillaoption.cpp \
    lookbackoption.cpp \
    multiassetoption.cpp \
    oneassetoption.cpp \
    oneassetstrikedoption.cpp \
    quantoforwardvanillaoption.cpp \
    quantovanillaoption.cpp \
    stock.cpp \
    swap.cpp \
    swaption.cpp \
    vanillaoption.cpp \
    vanillaswap.cpp \
    varianceswap.cpp \
    zerocouponbond.cpp

noinst_LTLIBRARIES = libInstruments.la

EXTRA_DIST = makefile.mak

