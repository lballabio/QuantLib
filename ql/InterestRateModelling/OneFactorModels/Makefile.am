
# $Id$

INCLUDES = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/InterestRateModelling/OneFactorModels

include_HEADERS = \
    blackdermanandtoy.hpp \
    blackkarasinski.hpp \
    hoandlee.hpp \
    hullandwhite.hpp \
    node.hpp \
    tree.hpp

EXTRA_DIST = makefile.mak

libOneFactorModels_la_SOURCES = \
    blackdermanandtoy.cpp \
    blackkarasinski.cpp \
    hoandlee.cpp \
    hullandwhite.cpp \
    tree.cpp

noinst_LTLIBRARIES = libOneFactorModels.la
