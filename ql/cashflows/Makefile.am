
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
        all.hpp \
        core.hpp \
        analysis.hpp \
        capflooredcoupon.hpp \
        cashflowvectors.hpp \
        cmscoupon.hpp \
        conundrumpricer.hpp \
        coupon.hpp \
        couponpricer.hpp \
        digitalcoupon.hpp \
        dividend.hpp \
        fixedratecoupon.hpp \
        floatingratecoupon.hpp \
        iborcoupon.hpp \
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
    digitalcoupon.cpp \
    dividend.cpp \
    floatingratecoupon.cpp \
    iborcoupon.cpp \
    shortfloatingcoupon.cpp \
    timebasket.cpp

noinst_LTLIBRARIES = libCashFlows.la

