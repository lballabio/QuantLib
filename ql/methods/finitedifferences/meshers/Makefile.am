
AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
	all.hpp \
    concentrating1dmesher.hpp \
    exponentialjump1dmesher.hpp \
    fdm1dmesher.hpp \
    fdmblackscholesmesher.hpp \
    fdmblackscholesmultistrikemesher.hpp \
    fdmcev1dmesher.hpp \
    fdmhestonvariancemesher.hpp \
    fdmmeshercomposite.hpp \
    fdmmesher.hpp \
    fdmsimpleprocess1dmesher.hpp \
    predefined1dmesher.hpp \
    uniform1dmesher.hpp \
    uniformgridmesher.hpp

cpp_files = \
    concentrating1dmesher.cpp \
    exponentialjump1dmesher.cpp \
    fdmblackscholesmesher.cpp \
    fdmblackscholesmultistrikemesher.cpp \
    fdmcev1dmesher.cpp \
    fdmhestonvariancemesher.cpp \
    fdmmeshercomposite.cpp \
    fdmsimpleprocess1dmesher.cpp \
    uniformgridmesher.cpp

if UNITY_BUILD

nodist_libFdmMeshers_la_SOURCES = unity.cpp

include ${top_srcdir}/mk/unity.mk

EXTRA_DIST = $(cpp_files)

else

libFdmMeshers_la_SOURCES = $(cpp_files)

endif

noinst_LTLIBRARIES = libFdmMeshers.la

include ${top_srcdir}/mk/all.mk

