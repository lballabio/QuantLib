
# $Id$

INCLUDES= -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/CashFlows
include_HEADERS = \
        coupon.hpp \
        cashflowvectors.hpp \
        fixedratecoupon.hpp \
        floatingratecoupon.hpp \
        simplecashflow.hpp

EXTRA_DIST=makefile.mak

libCashFlows_la_SOURCES = \
    cashflowvectors.cpp \
    floatingratecoupon.cpp

noinst_LTLIBRARIES = libCashFlows.la
