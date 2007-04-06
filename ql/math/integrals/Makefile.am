
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/math/integrals

include_HEADERS = \
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

