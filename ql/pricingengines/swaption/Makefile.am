
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
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

