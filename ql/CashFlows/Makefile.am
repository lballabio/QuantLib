
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/CashFlows

include_HEADERS = \
        all.hpp \
        core.hpp \
        basispointsensitivity.hpp \
        cashflowvectors.hpp \
        coupon.hpp \
        fixedratecoupon.hpp \
        floatingratecoupon.hpp \
        inarrearindexedcoupon.hpp \
        indexedcashflowvectors.hpp \
        indexedcoupon.hpp \
        parcoupon.hpp \
        shortfloatingcoupon.hpp \
        shortindexedcoupon.hpp \
        simplecashflow.hpp \
        timebasket.hpp \
        upfrontindexedcoupon.hpp

EXTRA_DIST=makefile.mak

libCashFlows_la_SOURCES = \
    basispointsensitivity.cpp \
    cashflowvectors.cpp \
    inarrearindexedcoupon.cpp \
    parcoupon.cpp \
    shortfloatingcoupon.cpp \
    timebasket.cpp

noinst_LTLIBRARIES = libCashFlows.la
