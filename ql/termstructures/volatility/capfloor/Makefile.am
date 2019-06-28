AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
    all.hpp \
    capfloortermvolatilitystructure.hpp \
    capfloortermvolcurve.hpp \
    capfloortermvolsurface.hpp \
    constantcapfloortermvol.hpp

cpp_files = \
    capfloortermvolatilitystructure.cpp \
    capfloortermvolcurve.cpp \
    capfloortermvolsurface.cpp \
    constantcapfloortermvol.cpp

if UNITY_BUILD

nodist_libCapFloorVol_la_SOURCES = unity.cpp

include ${top_srcdir}/mk/unity.mk

EXTRA_DIST = $(cpp_files)

else

libCapFloorVol_la_SOURCES = $(cpp_files)

endif

noinst_LTLIBRARIES = libCapFloorVol.la

include ${top_srcdir}/mk/all.mk

