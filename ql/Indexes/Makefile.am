
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/Indexes

include_HEADERS = \
    all.hpp \
    core.hpp \
    audlibor.hpp \
    cadlibor.hpp \
    cdor.hpp \
    chflibor.hpp \
    dkklibor.hpp \
    euribor.hpp \
    euriborswapfixa.hpp \
    eurlibor.hpp \
    eurliborswapfixa.hpp \
    eurliborswapfixb.hpp \
    gbplibor.hpp \
    jibar.hpp \
    jpylibor.hpp \
    nzdlibor.hpp \
    tibor.hpp \
    trlibor.hpp \
    usdlibor.hpp \
    zibor.hpp \
    xibor.hpp \
    libor.hpp \
    indexmanager.hpp \
    interestrateindex.hpp \
    swapindex.hpp

libIndexes_la_SOURCES = \
    xibor.cpp \
    libor.cpp \
    indexmanager.cpp \
    interestrateindex.cpp \
    swapindex.cpp

noinst_LTLIBRARIES = libIndexes.la

EXTRA_DIST = makefile.mak

