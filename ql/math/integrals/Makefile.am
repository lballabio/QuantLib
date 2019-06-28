AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
	all.hpp \
	discreteintegrals.hpp \
	filonintegral.hpp \
	gausslobattointegral.hpp \
	gaussianorthogonalpolynomial.hpp \
	gaussianquadratures.hpp \
	integral.hpp \
	kronrodintegral.hpp \
	segmentintegral.hpp \
	simpsonintegral.hpp \
	trapezoidintegral.hpp \
	twodimensionalintegral.hpp

cpp_files = \
	discreteintegrals.cpp \
	filonintegral.cpp \
	gausslobattointegral.cpp \
	gaussianorthogonalpolynomial.cpp \
	gaussianquadratures.cpp \
	integral.cpp \
	kronrodintegral.cpp \
	segmentintegral.cpp

if UNITY_BUILD

nodist_libIntegrals_la_SOURCES = unity.cpp

include ${top_srcdir}/mk/unity.mk

EXTRA_DIST = $(cpp_files)

else

libIntegrals_la_SOURCES = $(cpp_files)

endif

noinst_LTLIBRARIES = libIntegrals.la

include ${top_srcdir}/mk/all.mk

