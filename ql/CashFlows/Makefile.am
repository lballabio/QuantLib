
# $Id$

AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/CashFlows

include_HEADERS = \
        basispointsensitivity.hpp \
        cashflowvectors.hpp \
        coupon.hpp \
        fixedratecoupon.hpp \
        floatingratecoupon.hpp \
        inarrearindexedcoupon.hpp \
        indexcashflowvectors.hpp \
        indexedcoupon.hpp \
        parcoupon.hpp \
        shortfloatingcoupon.hpp \
        shortindexedcoupon.hpp \
        simplecashflow.hpp \
        upfrontindexedcoupon.hpp

EXTRA_DIST=makefile.mak

libCashFlows_la_SOURCES = \
    cashflowvectors.cpp \
    parcoupon.cpp \
    shortfloatingcoupon.cpp

noinst_LTLIBRARIES = libCashFlows.la
