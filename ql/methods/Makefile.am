
SUBDIRS = finitedifferences lattices montecarlo

AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
	all.hpp

libMethods_la_SOURCES =

libMethods_la_LIBADD = \
    finitedifferences/libFiniteDifferences.la \
    lattices/libLattices.la \
    montecarlo/libMonteCarlo.la

noinst_LTLIBRARIES = libMethods.la

include ${top_srcdir}/mk/all.mk

