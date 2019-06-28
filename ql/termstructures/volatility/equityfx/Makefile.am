
AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
    all.hpp \
    andreasenhugelocalvoladapter.hpp \
    andreasenhugevolatilityinterpl.hpp \
    andreasenhugevolatilityadapter.hpp \
    blackconstantvol.hpp \
    blackvariancecurve.hpp \
    blackvariancesurface.hpp \
    blackvoltermstructure.hpp \
    fixedlocalvolsurface.hpp \
    gridmodellocalvolsurface.hpp \
    hestonblackvolsurface.hpp \
    impliedvoltermstructure.hpp \
    localconstantvol.hpp \
    localvolcurve.hpp \
    localvolsurface.hpp \
    localvoltermstructure.hpp \
    noexceptlocalvolsurface.hpp

cpp_files = \
    andreasenhugelocalvoladapter.cpp \
    andreasenhugevolatilityinterpl.cpp \
    andreasenhugevolatilityadapter.cpp \
    blackvariancecurve.cpp \
    blackvariancesurface.cpp \
    blackvoltermstructure.cpp \
    fixedlocalvolsurface.cpp \
    gridmodellocalvolsurface.cpp \
    hestonblackvolsurface.cpp \
    localvolsurface.cpp \
    localvoltermstructure.cpp

if UNITY_BUILD

nodist_libEquityFxVol_la_SOURCES = unity.cpp

include ${top_srcdir}/mk/unity.mk

EXTRA_DIST = $(cpp_files)

else

libEquityFxVol_la_SOURCES = $(cpp_files)

endif

noinst_LTLIBRARIES = libEquityFxVol.la

include ${top_srcdir}/mk/all.mk

