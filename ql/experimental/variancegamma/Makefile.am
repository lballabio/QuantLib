
AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
    all.hpp \
    analyticvariancegammaengine.hpp \
    fftengine.hpp \
    fftvanillaengine.hpp \
    fftvariancegammaengine.hpp \
    variancegammamodel.hpp \
    variancegammaprocess.hpp

cpp_files = \
    analyticvariancegammaengine.cpp \
    fftengine.cpp \
    fftvanillaengine.cpp \
    fftvariancegammaengine.cpp \
    variancegammamodel.cpp \
    variancegammaprocess.cpp

if UNITY_BUILD

nodist_libVarianceGamma_la_SOURCES = unity.cpp

include ${top_srcdir}/mk/unity.mk

EXTRA_DIST = $(cpp_files)

else

libVarianceGamma_la_SOURCES = $(cpp_files)

endif

noinst_LTLIBRARIES = libVarianceGamma.la

include ${top_srcdir}/mk/all.mk

