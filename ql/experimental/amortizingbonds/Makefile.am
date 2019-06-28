
AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
    all.hpp \
    amortizingcmsratebond.hpp \
    amortizingfixedratebond.hpp \
    amortizingfloatingratebond.hpp

cpp_files = \
    amortizingcmsratebond.cpp \
    amortizingfixedratebond.cpp \
    amortizingfloatingratebond.cpp

if UNITY_BUILD

nodist_libAmortizingBonds_la_SOURCES = unity.cpp

include ${top_srcdir}/mk/unity.mk

EXTRA_DIST = $(cpp_files)

else

libAmortizingBonds_la_SOURCES = $(cpp_files)

endif

noinst_LTLIBRARIES = libAmortizingBonds.la

include ${top_srcdir}/mk/all.mk

