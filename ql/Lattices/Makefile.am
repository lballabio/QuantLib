
# $Id$

INCLUDES= -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/Lattices

include_HEADERS = \
    binomialtree.hpp \
    tree.hpp \
    trinomialtree.hpp

EXTRA_DIST=makefile.mak

libLattices_la_SOURCES = \
    binomialtree.cpp \
    tree.cpp \
    trinomialtree.cpp

noinst_LTLIBRARIES = libLattices.la
