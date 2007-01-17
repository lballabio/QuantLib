
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/CashFlows

include_HEADERS = \
        all.hpp \
        core.hpp \
        analysis.hpp \
        capflooredcoupon.hpp \
        capfloorlet.hpp \
        cashflowvectors.hpp \
        cmscoupon.hpp \
        conundrumpricer.hpp \
        coupon.hpp \
        dividend.hpp \
        fixedratecoupon.hpp \
        floatingratecoupon.hpp \
        inarrearindexedcoupon.hpp \
        indexedcashflowvectors.hpp \
        parcoupon.hpp \
        shortfloatingcoupon.hpp \
        shortindexedcoupon.hpp \
        simplecashflow.hpp \
        timebasket.hpp \
        upfrontindexedcoupon.hpp

libCashFlows_la_SOURCES = \
    analysis.cpp \
    capflooredcoupon.cpp \
    capfloorlet.cpp \
    cashflowvectors.cpp \
    cmscoupon.cpp \
    conundrumpricer.cpp \
    dividend.cpp \
    floatingratecoupon.cpp \
    inarrearindexedcoupon.cpp \
    parcoupon.cpp \
    shortfloatingcoupon.cpp \
    timebasket.cpp

noinst_LTLIBRARIES = libCashFlows.la

