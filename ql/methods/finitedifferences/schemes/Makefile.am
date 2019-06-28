
AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
	all.hpp \
	boundaryconditionschemehelper.hpp \
	craigsneydscheme.hpp \
	douglasscheme.hpp \
	expliciteulerscheme.hpp \
	hundsdorferscheme.hpp \
	impliciteulerscheme.hpp \
	methodoflinesscheme.hpp \
	modifiedcraigsneydscheme.hpp \
	trbdf2scheme.hpp

cpp_files = \
	craigsneydscheme.cpp \
	douglasscheme.cpp \
	expliciteulerscheme.cpp \
	hundsdorferscheme.cpp \
	impliciteulerscheme.cpp \
	methodoflinesscheme.cpp \
	modifiedcraigsneydscheme.cpp

if UNITY_BUILD

nodist_libFdmSchemes_la_SOURCES = unity.cpp

include ${top_srcdir}/mk/unity.mk

EXTRA_DIST = $(cpp_files)

else

libFdmSchemes_la_SOURCES = $(cpp_files)

endif

noinst_LTLIBRARIES = libFdmSchemes.la

include ${top_srcdir}/mk/all.mk

