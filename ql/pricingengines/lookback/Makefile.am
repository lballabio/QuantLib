
AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
	all.hpp \
	analyticcontinuousfixedlookback.hpp \
	analyticcontinuousfloatinglookback.hpp \
	analyticcontinuouspartialfixedlookback.hpp \
	analyticcontinuouspartialfloatinglookback.hpp

cpp_files = \
	analyticcontinuousfixedlookback.cpp \
	analyticcontinuousfloatinglookback.cpp \
	analyticcontinuouspartialfixedlookback.cpp \
	analyticcontinuouspartialfloatinglookback.cpp

if UNITY_BUILD

nodist_libLookbackEngines_la_SOURCES = unity.cpp

include ${top_srcdir}/mk/unity.mk

EXTRA_DIST = $(cpp_files)

else

libLookbackEngines_la_SOURCES = $(cpp_files)

endif

noinst_LTLIBRARIES = libLookbackEngines.la

include ${top_srcdir}/mk/all.mk

