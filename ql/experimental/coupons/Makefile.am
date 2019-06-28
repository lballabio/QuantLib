
AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
    all.hpp \
    cmsspreadcoupon.hpp \
    digitalcmsspreadcoupon.hpp \
    lognormalcmsspreadpricer.hpp \
    proxyibor.hpp \
    quantocouponpricer.hpp \
    strippedcapflooredcoupon.hpp \
    subperiodcoupons.hpp \
    swapspreadindex.hpp

cpp_files = \
    cmsspreadcoupon.cpp \
    digitalcmsspreadcoupon.cpp \
    lognormalcmsspreadpricer.cpp \
    proxyibor.cpp \
    quantocouponpricer.cpp \
    strippedcapflooredcoupon.cpp \
    subperiodcoupons.cpp \
    swapspreadindex.cpp

if UNITY_BUILD

nodist_libCoupons_la_SOURCES = unity.cpp

include ${top_srcdir}/mk/unity.mk

EXTRA_DIST = $(cpp_files)

else

libCoupons_la_SOURCES = $(cpp_files)

endif

noinst_LTLIBRARIES = libCoupons.la

include ${top_srcdir}/mk/all.mk

