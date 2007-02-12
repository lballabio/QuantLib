
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/MarketModels/DriftComputation

include_HEADERS = \
	cmsmmdriftcalculator.hpp \
	lmmdriftcalculator.hpp 
#	smmdriftcalculator.hpp

libMarketModelsDriftComputation_la_SOURCES = \
	cmsmmdriftcalculator.cpp \
	lmmdriftcalculator.cpp 
#	smmdriftcalculator.cpp

noinst_LTLIBRARIES = libMarketModelsDriftComputation.la

