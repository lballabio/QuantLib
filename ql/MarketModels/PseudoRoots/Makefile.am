
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/MarketModels/PseudoRoots

include_HEADERS = \
	abcd.hpp \
	abcdfit.hpp \
	calibratedmarketmodel.hpp \
	expcorrabcdvol.hpp \
	expcorrflatvol.hpp

libMarketModelsPseudoRoots_la_SOURCES = \
	abcd.cpp \
	calibratedmarketmodel.cpp \
	expcorrabcdvol.cpp \
	expcorrflatvol.cpp

noinst_LTLIBRARIES = libMarketModelsPseudoRoots.la

EXTRA_DIST = makefile.mak

