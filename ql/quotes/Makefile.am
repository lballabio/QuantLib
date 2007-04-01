
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/quotes

include_HEADERS = \
    all.hpp \
    compositequote.hpp \
    derivedquote.hpp \
    futuresconvadjustmentquote.hpp \
    simplequote.hpp

libQuotes_la_SOURCES = \
    derivedquote.cpp \
    futuresconvadjustmentquote.cpp

noinst_LTLIBRARIES = libQuotes.la


