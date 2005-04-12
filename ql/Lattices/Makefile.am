
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/Lattices

include_HEADERS = \
    all.hpp \
    core.hpp \
    binomialtree.hpp \
    bsmlattice.hpp \
    lattice.hpp \
    lattice1d.hpp \
    lattice2d.hpp \
    tree.hpp \
    trinomialtree.hpp

EXTRA_DIST=makefile.mak

libLattices_la_SOURCES = \
    binomialtree.cpp \
    trinomialtree.cpp

noinst_LTLIBRARIES = libLattices.la
