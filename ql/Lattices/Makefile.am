
# $Id$

INCLUDES= -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/Lattices

include_HEADERS = \
    column.hpp \
    tree.hpp \
    trinomialtree.hpp \
    twodimensionaltree.hpp

EXTRA_DIST=makefile.mak

libLattices_la_SOURCES = \
    tree.cpp \
    trinomialtree.cpp \
    twodimensionaltree.cpp

noinst_LTLIBRARIES = libLattices.la
