
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/RandomNumbers

include_HEADERS = \
	all.hpp \
	core.hpp \
	boxmullergaussianrng.hpp \
	centrallimitgaussianrng.hpp \
	faurersg.hpp \
	haltonrsg.hpp \
	inversecumulativerng.hpp \
	inversecumulativersg.hpp \
	knuthuniformrng.hpp \
	lecuyeruniformrng.hpp \
	mt19937uniformrng.hpp \
	primitivepolynomials.h \
	randomizedlds.hpp \
	randomsequencegenerator.hpp \
	rngtraits.hpp \
	seedgenerator.hpp \
	sobolrsg.hpp

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

EXTRA_DIST = makefile.mak

