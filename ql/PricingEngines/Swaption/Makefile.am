
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/PricingEngines/Swaption

include_HEADERS = \
    all.hpp \
    blackswaptionengine.hpp \
    g2swaptionengine.hpp \
    jamshidianswaptionengine.hpp \
    discretizedswaption.hpp \
    lfmswaptionengine.hpp \
    treeswaptionengine.hpp

libSwaptionEngines_la_SOURCES = \
    blackswaptionengine.cpp \
    jamshidianswaptionengine.cpp \
    discretizedswaption.cpp \
    lfmswaptionengine.cpp \
    treeswaptionengine.cpp

noinst_LTLIBRARIES = libSwaptionEngines.la

EXTRA_DIST = makefile.mak

