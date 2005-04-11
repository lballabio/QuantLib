
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/Indexes

include_HEADERS = \
    all.hpp \
    core.hpp \
    audlibor.hpp \
    cadlibor.hpp \
    cdor.hpp \
    chflibor.hpp \
    euribor.hpp \
    gbplibor.hpp \
    jpylibor.hpp \
    tibor.hpp \
    usdlibor.hpp \
    zarlibor.hpp \
    zibor.hpp \
    xibor.hpp \
    indexmanager.hpp

EXTRA_DIST=makefile.mak

libIndexes_la_SOURCES = \
    xibor.cpp \
    indexmanager.cpp

noinst_LTLIBRARIES = libIndexes.la
