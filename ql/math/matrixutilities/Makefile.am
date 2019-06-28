
AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
	all.hpp \
	basisincompleteordered.hpp \
	bicgstab.hpp \
	choleskydecomposition.hpp \
	factorreduction.hpp \
	getcovariance.hpp \
	gmres.hpp \
	pseudosqrt.hpp \
	qrdecomposition.hpp \
	sparseilupreconditioner.hpp \
	sparsematrix.hpp \
	svd.hpp \
	symmetricschurdecomposition.hpp \
	tapcorrelations.hpp \
	tqreigendecomposition.hpp

cpp_files = \
	bicgstab.cpp \
	basisincompleteordered.cpp \
	choleskydecomposition.cpp \
	factorreduction.cpp \
	getcovariance.cpp \
	gmres.cpp \
	pseudosqrt.cpp \
	qrdecomposition.cpp \
	sparseilupreconditioner.cpp \
	svd.cpp \
	symmetricschurdecomposition.cpp \
	tapcorrelations.cpp \
	tqreigendecomposition.cpp

if UNITY_BUILD

nodist_libMatrixUtilities_la_SOURCES = unity.cpp

include ${top_srcdir}/mk/unity.mk

EXTRA_DIST = $(cpp_files)

else

libMatrixUtilities_la_SOURCES = $(cpp_files)

endif

noinst_LTLIBRARIES = libMatrixUtilities.la

include ${top_srcdir}/mk/all.mk

