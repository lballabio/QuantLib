
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/Utilities

include_HEADERS = \
    all.hpp \
    dataformatters.hpp \
    dataparsers.hpp \
    disposable.hpp \
    null.hpp \
    observablevalue.hpp \
    steppingiterator.hpp \
    strings.hpp \
    tracing.hpp

libUtilities_la_SOURCES = \
    dataformatters.cpp \
    dataparsers.cpp \
    strings.cpp \
    tracing.cpp

noinst_LTLIBRARIES = libUtilities.la

EXTRA_DIST = makefile.mak

