
AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
    all.hpp \
    batesmodel.hpp \
    gjrgarchmodel.hpp \
    hestonmodel.hpp \
    hestonmodelhelper.hpp \
    piecewisetimedependenthestonmodel.hpp

cpp_files = \
    batesmodel.cpp \
    gjrgarchmodel.cpp \
    hestonmodel.cpp \
    hestonmodelhelper.cpp \
    piecewisetimedependenthestonmodel.cpp

if UNITY_BUILD

nodist_libEquityModels_la_SOURCES = unity.cpp

include ${top_srcdir}/mk/unity.mk

EXTRA_DIST = $(cpp_files)

else

libEquityModels_la_SOURCES = $(cpp_files)

endif

noinst_LTLIBRARIES = libEquityModels.la

include ${top_srcdir}/mk/all.mk

