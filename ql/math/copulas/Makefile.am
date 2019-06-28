AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
	all.hpp \
	alimikhailhaqcopula.hpp \
	claytoncopula.hpp \
	farliegumbelmorgensterncopula.hpp \
	frankcopula.hpp \
	galamboscopula.hpp \
	gaussiancopula.hpp \
	gumbelcopula.hpp \
	huslerreisscopula.hpp \
	independentcopula.hpp \
	marshallolkincopula.hpp \
	maxcopula.hpp \
	mincopula.hpp \
	plackettcopula.hpp

cpp_files = \
	alimikhailhaqcopula.cpp \
	claytoncopula.cpp \
	farliegumbelmorgensterncopula.cpp \
	frankcopula.cpp \
	galamboscopula.cpp \
	gaussiancopula.cpp \
	gumbelcopula.cpp \
	huslerreisscopula.cpp \
	independentcopula.cpp \
	marshallolkincopula.cpp \
	maxcopula.cpp \
	mincopula.cpp \
	plackettcopula.cpp

if UNITY_BUILD

nodist_libCopulas_la_SOURCES = unity.cpp

include ${top_srcdir}/mk/unity.mk

EXTRA_DIST = $(cpp_files)

else

libCopulas_la_SOURCES = $(cpp_files)

endif

noinst_LTLIBRARIES = libCopulas.la

include ${top_srcdir}/mk/all.mk

