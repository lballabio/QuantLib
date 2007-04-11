
SUBDIRS = finitedifferences lattices montecarlo

AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
	all.hpp

libMethods_la_SOURCES =

libMethods_la_LIBADD = \
    finitedifferences/libFiniteDifferences.la \
    lattices/libLattices.la \
    montecarlo/libMonteCarlo.la

noinst_LTLIBRARIES = libMethods.la

