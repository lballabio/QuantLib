# $Id$

INCLUDES= -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/Minimizers

SUBDIRS = GeneticAlgorithms

include_HEADERS = \
    bfgs.hpp \
    brent.hpp \
    cubic.hpp \
	genetic.hpp \
    linesearch.hpp \
    powell.hpp

EXTRA_DIST=makefile.mak

libMinimizers_la_SOURCES = \
    bfgs.cpp \
    brent.cpp \
    cubic.cpp \
	genetic.cpp \
    linesearch.cpp \
    powell.cpp

noinst_LTLIBRARIES = libMinimizers.la
