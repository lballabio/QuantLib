
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/PricingEngines/Swaption

include_HEADERS = \
    all.hpp \
    blackswaption.hpp \
    jamshidianswaption.hpp \
    swaptionpricer.hpp \
    treeswaption.hpp

EXTRA_DIST=makefile.mak

libSwaptionEngines_la_SOURCES = \
    blackswaption.cpp \
    jamshidianswaption.cpp \
    swaptionpricer.cpp \
    treeswaption.cpp

noinst_LTLIBRARIES = libSwaptionEngines.la
