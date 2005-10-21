
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
    eurlibor.hpp \
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
    indexmanager.hpp

libIndexes_la_SOURCES = \
    xibor.cpp \
    libor.cpp \
    indexmanager.cpp

noinst_LTLIBRARIES = libIndexes.la

EXTRA_DIST = makefile.mak

