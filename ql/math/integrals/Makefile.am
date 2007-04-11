
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
	all.hpp \
	gaussianorthogonalpolynomial.hpp \
	gaussianquadratures.hpp \
	integral.hpp \
	kronrodintegral.hpp \
	segmentintegral.hpp \
	simpsonintegral.hpp \
	trapezoidintegral.hpp

libIntegrals_la_SOURCES = \
	gaussianorthogonalpolynomial.cpp \
	gaussianquadratures.cpp \
	kronrodintegral.cpp \
	integral.cpp

noinst_LTLIBRARIES = libIntegrals.la

