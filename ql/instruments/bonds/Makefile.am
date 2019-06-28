
AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
    all.hpp \
    btp.hpp \
    cmsratebond.hpp \
    cpibond.hpp \
    fixedratebond.hpp \
    floatingratebond.hpp \
    zerocouponbond.hpp

cpp_files = \
    btp.cpp \
    cmsratebond.cpp \
    cpibond.cpp \
    fixedratebond.cpp \
    floatingratebond.cpp \
    zerocouponbond.cpp

if UNITY_BUILD

nodist_libBonds_la_SOURCES = unity.cpp

include ${top_srcdir}/mk/unity.mk

EXTRA_DIST = $(cpp_files)

else

libBonds_la_SOURCES = $(cpp_files)

endif

noinst_LTLIBRARIES = libBonds.la

include ${top_srcdir}/mk/all.mk

