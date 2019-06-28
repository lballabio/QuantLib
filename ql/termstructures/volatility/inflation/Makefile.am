AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
    all.hpp \
    constantcpivolatility.hpp \
    cpivolatilitystructure.hpp \
    yoyinflationoptionletvolatilitystructure.hpp

cpp_files = \
    constantcpivolatility.cpp \
    cpivolatilitystructure.cpp \
    yoyinflationoptionletvolatilitystructure.cpp

if UNITY_BUILD

nodist_libInflationVol_la_SOURCES = unity.cpp

include ${top_srcdir}/mk/unity.mk

EXTRA_DIST = $(cpp_files)

else

libInflationVol_la_SOURCES = $(cpp_files)

endif

noinst_LTLIBRARIES = libInflationVol.la

include ${top_srcdir}/mk/all.mk

