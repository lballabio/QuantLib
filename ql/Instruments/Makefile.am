
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/Instruments

include_HEADERS = \
    barrieroption.hpp \
    binaryoption.hpp \
    capfloor.hpp \
    cliquetoption.hpp \
    forwardvanillaoption.hpp \
    quantoforwardvanillaoption.hpp \
    quantovanillaoption.hpp \
    simpleswap.hpp \
    stock.hpp \
    swap.hpp \
    swaption.hpp \
    vanillaoption.hpp

EXTRA_DIST=makefile.mak

libInstruments_la_SOURCES = \
    barrieroption.cpp \
    binaryoption.cpp \
    capfloor.cpp \
    forwardvanillaoption.cpp \
    quantoforwardvanillaoption.cpp \
    quantovanillaoption.cpp \
    simpleswap.cpp \
    stock.cpp \
    swap.cpp \
    swaption.cpp \
    vanillaoption.cpp

noinst_LTLIBRARIES = libInstruments.la
