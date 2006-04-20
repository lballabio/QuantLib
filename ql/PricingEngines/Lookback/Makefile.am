
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/PricingEngines/Lookback

include_HEADERS = \
	all.hpp \
	analyticcontinuousfixedlookback.hpp \
	analyticcontinuousfloatinglookback.hpp

libLookbackEngines_la_SOURCES = \
	analyticcontinuousfixedlookback.cpp \
	analyticcontinuousfloatinglookback.cpp

noinst_LTLIBRARIES = libLookbackEngines.la

EXTRA_DIST = makefile.mak

