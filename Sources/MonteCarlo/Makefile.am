
EXTRA_DIST=makefile.mak

INCLUDES = -I$(top_srcdir)/Include/ \
	-I$(top_srcdir)/Include/MonteCarlo \
	-I$(top_srcdir)/Include/Math \
	-I$(top_srcdir)/Include/Utilities

libMonteCarlo_la_SOURCES = \
	avgstrikeasianpathpricer.cpp \
	avgpriceasianpathpricer.cpp \
	basketpathpricer.cpp    \
	controlvariatedpathpricer.cpp \
	everestpathpricer.cpp \
	europeanpathpricer.cpp \
	geometricasianpathpricer.cpp \
	getcovariance.cpp \
	himalayapathpricer.cpp \
	pagodapathpricer.cpp \
	lecuyerrandomgenerator.cpp


noinst_LTLIBRARIES = libMonteCarlo.la
