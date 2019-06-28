
AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
    all.hpp \
    capletvariancecurve.hpp \
    constantoptionletvol.hpp \
    optionletstripper.hpp \
    optionletstripper1.hpp \
    optionletstripper2.hpp \
    optionletvolatilitystructure.hpp \
    spreadedoptionletvol.hpp \
    strippedoptionlet.hpp \
    strippedoptionletadapter.hpp \
    strippedoptionletbase.hpp

cpp_files = \
    constantoptionletvol.cpp \
    optionletstripper.cpp \
    optionletstripper1.cpp \
    optionletstripper2.cpp \
    optionletvolatilitystructure.cpp \
    spreadedoptionletvol.cpp \
    strippedoptionlet.cpp \
    strippedoptionletadapter.cpp

if UNITY_BUILD

nodist_libOptionletVol_la_SOURCES = unity.cpp

include ${top_srcdir}/mk/unity.mk

EXTRA_DIST = $(cpp_files)

else

libOptionletVol_la_SOURCES = $(cpp_files)

endif

noinst_LTLIBRARIES = libOptionletVol.la

include ${top_srcdir}/mk/all.mk

