INCLUDES = -I$(top_srcdir)/Include \
	-I$(top_srcdir)/Include/Pricers \
	-I$(top_srcdir)/Include/FiniteDifferences \
	-I$(top_srcdir)/Include/Math \
	-I$(top_srcdir)/Include/Solvers1D \
	-I$(top_srcdir)/Include/MonteCarlo \
	-I$(top_srcdir)/Include/Utilities

libPricers_la_SOURCES = \
    averagepriceasian.cpp \
    averagestrikeasian.cpp \
    bsmamericanoption.cpp \
    bsmeuropeanoption.cpp \
    bsmnumericaloption.cpp \
    bsmoption.cpp \
    dividendamericanoption.cpp \
    dividendeuropeanoption.cpp \
    himalaya.cpp \
    mceuropeanpricer.cpp \
    plainbasketoption.cpp

noinst_LTLIBRARIES = libPricers.la
