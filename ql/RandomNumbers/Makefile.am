
# $Id$

INCLUDES= -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/RNG

include_HEADERS = \
	boxmuller.hpp \
	centrallimitgaussian.hpp \
	inversecumulativegaussian.hpp \
	knuthrandomgenerator.hpp \
	lecuyerrandomgenerator.hpp \
	randomarraygenerator.hpp \
	rngtypedefs.hpp

EXTRA_DIST=makefile.mak

libRandomNumbers_la_SOURCES = \
	knuthrandomgenerator.cpp \
	lecuyerrandomgenerator.cpp

noinst_LTLIBRARIES = libRandomNumbers.la
