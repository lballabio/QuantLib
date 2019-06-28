
AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
	all.hpp \
	brownianbridge.hpp \
	earlyexercisepathpricer.hpp \
	exercisestrategy.hpp \
	genericlsregression.hpp \
	longstaffschwartzpathpricer.hpp \
	lsmbasissystem.hpp \
	mctraits.hpp \
	montecarlomodel.hpp \
	multipath.hpp \
	multipathgenerator.hpp \
	nodedata.hpp \
	parametricexercise.hpp \
	path.hpp \
	pathgenerator.hpp \
	pathpricer.hpp \
	sample.hpp

cpp_files = \
	brownianbridge.cpp \
	genericlsregression.cpp \
	lsmbasissystem.cpp \
	parametricexercise.cpp

if UNITY_BUILD

nodist_libMonteCarlo_la_SOURCES = unity.cpp

include ${top_srcdir}/mk/unity.mk

EXTRA_DIST = $(cpp_files)

else

libMonteCarlo_la_SOURCES = $(cpp_files)

endif

noinst_LTLIBRARIES = libMonteCarlo.la

include ${top_srcdir}/mk/all.mk

