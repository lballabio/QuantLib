
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/RandomNumbers

include_HEADERS = \
	all.hpp \
	core.hpp \
	boxmullergaussianrng.hpp \
	centrallimitgaussianrng.hpp \
	faurersg.hpp \
	haltonrsg.hpp \
	inversecumgaussianrng.hpp \
	inversecumgaussianrsg.hpp \
	knuthuniformrng.hpp \
	lecuyeruniformrng.hpp \
	mt19937uniformrng.hpp \
	primitivepolynomials.h \
	randomsequencegenerator.hpp \
	rngtraits.hpp \
	seedgenerator.hpp \
	sobolrsg.hpp

EXTRA_DIST=makefile.mak

libRandomNumbers_la_SOURCES = \
    faurersg.cpp \
    haltonrsg.cpp \
	knuthuniformrng.cpp \
	lecuyeruniformrng.cpp \
	mt19937uniformrng.cpp \
	primitivepolynomials.c \
	seedgenerator.cpp \
	sobolrsg.cpp

noinst_LTLIBRARIES = libRandomNumbers.la
