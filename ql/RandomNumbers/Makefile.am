
# $Id$

AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/RandomNumbers

include_HEADERS = \
	boxmullergaussianrng.hpp \
	centrallimitgaussianrng.hpp \
	haltonrsg.hpp \
	inversecumgaussianrng.hpp \
	inversecumgaussianrsg.hpp \
	knuthuniformrng.hpp \
	lecuyeruniformrng.hpp \
	randomarraygenerator.hpp \
	randomsequencegenerator.hpp \
	rngtypedefs.hpp

EXTRA_DIST=makefile.mak

libRandomNumbers_la_SOURCES = \
	haltonrsg.cpp \
	knuthuniformrng.cpp \
	lecuyeruniformrng.cpp

noinst_LTLIBRARIES = libRandomNumbers.la
