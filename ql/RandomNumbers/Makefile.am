
# $Id$

AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/RandomNumbers

include_HEADERS = \
	boxmullergaussianrng.hpp \
	centrallimitgaussianrng.hpp \
	halton.hpp \
	inversecumgaussianrng.hpp \
	knuthuniformrng.hpp \
	lecuyeruniformrng.hpp \
	randomarraygenerator.hpp \
	rngtypedefs.hpp

EXTRA_DIST=makefile.mak

libRandomNumbers_la_SOURCES = \
	halton.cpp \
	knuthuniformrng.cpp \
	lecuyeruniformrng.cpp

noinst_LTLIBRARIES = libRandomNumbers.la
