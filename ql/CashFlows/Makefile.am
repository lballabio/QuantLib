
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/CashFlows

include_HEADERS = \
        all.hpp \
        core.hpp \
        analysis.hpp \
        capflooredcoupon.hpp \
        cashflowvectors.hpp \
        cmscoupon.hpp \
        conundrumpricer.hpp \
        coupon.hpp \
        dividend.hpp \
        fixedratecoupon.hpp \
        floatingratecoupon.hpp \
        shortfloatingcoupon.hpp \
        shortindexedcoupon.hpp \
        simplecashflow.hpp \
        timebasket.hpp

libCashFlows_la_SOURCES = \
    analysis.cpp \
    capflooredcoupon.cpp \
    cashflowvectors.cpp \
    cmscoupon.cpp \
    conundrumpricer.cpp \
    couponpricer.cpp \
    dividend.cpp \
    floatingratecoupon.cpp \
    shortfloatingcoupon.cpp \
    timebasket.cpp

noinst_LTLIBRARIES = libCashFlows.la

