
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/Currencies

include_HEADERS = \
    all.hpp \
    exchangerate.hpp \
    money.hpp \
    ratemanager.hpp \
    audcurrency.hpp \
    cadcurrency.hpp \
    chfcurrency.hpp \
    demcurrency.hpp \
    eurcurrency.hpp \
    gbpcurrency.hpp \
    itlcurrency.hpp \
    jpycurrency.hpp \
    usdcurrency.hpp \
    zarcurrency.hpp

EXTRA_DIST=makefile.mak

libCurrencies_la_SOURCES = \
    exchangerate.cpp \
    money.cpp \
    ratemanager.cpp \
    audcurrency.cpp \
    cadcurrency.cpp \
    chfcurrency.cpp \
    demcurrency.cpp \
    eurcurrency.cpp \
    gbpcurrency.cpp \
    itlcurrency.cpp \
    jpycurrency.cpp \
    usdcurrency.cpp \
    zarcurrency.cpp

noinst_LTLIBRARIES = libCurrencies.la
