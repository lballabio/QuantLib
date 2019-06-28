
AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
    all.hpp \
    cpicapfloorengines.hpp \
    cpicapfloortermpricesurface.hpp \
    genericindexes.hpp \
    interpolatedyoyoptionletstripper.hpp \
    kinterpolatedyoyoptionletvolatilitysurface.hpp \
    piecewiseyoyoptionletvolatility.hpp \
    polynomial2Dspline.hpp \
    yoycapfloortermpricesurface.hpp \
    yoyoptionlethelpers.hpp \
    yoyoptionletstripper.hpp \
    yoyinflationoptionletvolatilitystructure2.hpp

cpp_files = \
    cpicapfloorengines.cpp \
    cpicapfloortermpricesurface.cpp \
    yoycapfloortermpricesurface.cpp \
    yoyoptionlethelpers.cpp

if UNITY_BUILD

nodist_libInflation_la_SOURCES = unity.cpp

include ${top_srcdir}/mk/unity.mk

EXTRA_DIST = $(cpp_files)

else

libInflation_la_SOURCES = $(cpp_files)

endif

noinst_LTLIBRARIES = libInflation.la

include ${top_srcdir}/mk/all.mk

