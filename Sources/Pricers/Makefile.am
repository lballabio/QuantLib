INCLUDES = -I$(top_srcdir)/Include/ \
	-I$(top_srcdir)/Include/Pricers \
	-I$(top_srcdir)/Include/FiniteDifferences \
	-I$(top_srcdir)/Include/Math \
	-I$(top_srcdir)/Include/Solvers1D \
	-I$(top_srcdir)/Include/MonteCarlo

libPricers_la_SOURCES = bsmamericanoption.cpp \
    bsmeuropeanoption.cpp \
    bsmnumericaloption.cpp \
    bsmoption.cpp \
    dividendamericanoption.cpp \
	mcasianpricer.cpp \
	mceuropeanpricer.cpp \
	plainbasketoption.cpp \

noinst_LTLIBRARIES = libPricers.la
