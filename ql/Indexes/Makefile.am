
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/Indexes

include_HEADERS = \
    all.hpp \
    core.hpp \
    audlibor.hpp \
    cadlibor.hpp \
    chflibor.hpp \
    euribor.hpp \
    gbplibor.hpp \
    usdlibor.hpp \
    jpylibor.hpp \
    zarlibor.hpp \
    xibor.hpp \
    indexmanager.hpp \
    xibormanager.hpp

EXTRA_DIST=makefile.mak

libIndexes_la_SOURCES = \
    xibor.cpp \
    indexmanager.cpp \
    xibormanager.cpp

noinst_LTLIBRARIES = libIndexes.la
