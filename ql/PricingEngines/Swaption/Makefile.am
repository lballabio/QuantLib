
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/PricingEngines/Swaption

include_HEADERS = \
    all.hpp \
    blackswaptionengine.hpp \
    g2swaptionengine.hpp \
    jamshidianswaptionengine.hpp \
    discretizedswaption.hpp \
    treeswaptionengine.hpp

EXTRA_DIST=makefile.mak

libSwaptionEngines_la_SOURCES = \
    blackswaptionengine.cpp \
    jamshidianswaptionengine.cpp \
    discretizedswaption.cpp \
    treeswaptionengine.cpp

noinst_LTLIBRARIES = libSwaptionEngines.la
