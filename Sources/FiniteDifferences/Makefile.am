AUTOMAKE_OPTIONS=foreign

EXTRA_DIST=makefile.mak

INCLUDES = -I$(top_srcdir)/Include \
	-I$(top_srcdir)/Include/FiniteDifferences \
	-I$(top_srcdir)/Include/Pricers

libFiniteDifferences_la_SOURCES = bsmoperator.cpp \
        tridiagonaloperator.cpp \
        valueatcenter.cpp

noinst_LTLIBRARIES = libFiniteDifferences.la
