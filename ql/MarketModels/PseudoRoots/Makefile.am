
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/MarketModels/PseudoRoots

include_HEADERS = \
	abcdvolatility.hpp \
	exponentialcorrelation.hpp

libMarketModelsPseudoRoots_la_SOURCES = \
	abcdvolatility.cpp \
	exponentialcorrelation.cpp

noinst_LTLIBRARIES = libMarketModelsPseudoRoots.la

EXTRA_DIST = makefile.mak

