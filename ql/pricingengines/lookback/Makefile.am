
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
	all.hpp \
	analyticcontinuousfixedlookback.hpp \
	analyticcontinuousfloatinglookback.hpp

libLookbackEngines_la_SOURCES = \
	analyticcontinuousfixedlookback.cpp \
	analyticcontinuousfloatinglookback.cpp

noinst_LTLIBRARIES = libLookbackEngines.la

