
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/marketmodels/driftcomputation

include_HEADERS = \
	all.hpp \
	cmsmmdriftcalculator.hpp \
	lmmdriftcalculator.hpp \
	lmmnormaldriftcalculator.hpp \
	smmdriftcalculator.hpp

libMarketModelsDriftComputation_la_SOURCES = \
	cmsmmdriftcalculator.cpp \
	lmmdriftcalculator.cpp \
	lmmnormaldriftcalculator.cpp \
	smmdriftcalculator.cpp

noinst_LTLIBRARIES = libMarketModelsDriftComputation.la

