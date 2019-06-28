
AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
	all.hpp \
	fdm2dblackscholessolver.hpp \
	fdm1dimsolver.hpp \
	fdm2dimsolver.hpp \
	fdm3dimsolver.hpp \
	fdmbackwardsolver.hpp \
	fdmbatessolver.hpp \
	fdmblackscholessolver.hpp \
	fdmg2solver.hpp \
	fdmhestonhullwhitesolver.hpp \
	fdmhestonsolver.hpp \
	fdmhullwhitesolver.hpp \
	fdmndimsolver.hpp \
	fdmsimple2dbssolver.hpp \
	fdmsolverdesc.hpp

cpp_files = \
	fdm2dblackscholessolver.cpp \
	fdm1dimsolver.cpp \
	fdm2dimsolver.cpp \
	fdm3dimsolver.cpp \
	fdmbackwardsolver.cpp \
	fdmbatessolver.cpp \
	fdmblackscholessolver.cpp \
	fdmg2solver.cpp \
	fdmhestonhullwhitesolver.cpp \
	fdmhestonsolver.cpp \
	fdmhullwhitesolver.cpp \
	fdmsimple2dbssolver.cpp

if UNITY_BUILD

nodist_libFdmSolvers_la_SOURCES = unity.cpp

include ${top_srcdir}/mk/unity.mk

EXTRA_DIST = $(cpp_files)

else

libFdmSolvers_la_SOURCES = $(cpp_files)

endif

noinst_LTLIBRARIES = libFdmSolvers.la

include ${top_srcdir}/mk/all.mk

