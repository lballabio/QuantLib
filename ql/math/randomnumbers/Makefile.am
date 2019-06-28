
AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
	all.hpp \
	boxmullergaussianrng.hpp \
	centrallimitgaussianrng.hpp \
	faurersg.hpp \
	haltonrsg.hpp \
	inversecumulativerng.hpp \
	inversecumulativersg.hpp \
	knuthuniformrng.hpp \
	latticersg.hpp \
	latticerules.hpp \
	lecuyeruniformrng.hpp \
	mt19937uniformrng.hpp \
	primitivepolynomials.hpp \
	randomizedlds.hpp \
	randomsequencegenerator.hpp \
	ranluxuniformrng.hpp \
	rngtraits.hpp \
	seedgenerator.hpp \
	sobolbrownianbridgersg.hpp \
	sobolrsg.hpp \
	stochasticcollocationinvcdf.hpp

cpp_files = \
    faurersg.cpp \
    haltonrsg.cpp \
	knuthuniformrng.cpp \
	latticersg.cpp \
	latticerules.cpp \
	lecuyeruniformrng.cpp \
	mt19937uniformrng.cpp \
	primitivepolynomials.cpp \
	seedgenerator.cpp \
	sobolbrownianbridgersg.cpp \
	sobolrsg.cpp \
	stochasticcollocationinvcdf.cpp

if UNITY_BUILD

nodist_libRandomNumbers_la_SOURCES = unity.cpp

include ${top_srcdir}/mk/unity.mk

EXTRA_DIST = $(cpp_files)

else

libRandomNumbers_la_SOURCES = $(cpp_files)

endif

noinst_LTLIBRARIES = libRandomNumbers.la

include ${top_srcdir}/mk/all.mk

