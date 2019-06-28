
AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
    all.hpp \
    basketgeneratingengine.hpp \
    blackswaptionengine.hpp \
    discretizedswaption.hpp \
    gaussian1dfloatfloatswaptionengine.hpp \
    gaussian1djamshidianswaptionengine.hpp \
    gaussian1dnonstandardswaptionengine.hpp \
    gaussian1dswaptionengine.hpp \
    g2swaptionengine.hpp \
    jamshidianswaptionengine.hpp \
    fdg2swaptionengine.hpp \
    fdhullwhiteswaptionengine.hpp \
    treeswaptionengine.hpp

cpp_files = \
    basketgeneratingengine.cpp \
    blackswaptionengine.cpp \
    discretizedswaption.cpp \
    gaussian1dfloatfloatswaptionengine.cpp \
    gaussian1djamshidianswaptionengine.cpp \
    gaussian1dnonstandardswaptionengine.cpp \
    gaussian1dswaptionengine.cpp \
    jamshidianswaptionengine.cpp \
    fdg2swaptionengine.cpp \
    fdhullwhiteswaptionengine.cpp \
    treeswaptionengine.cpp

if UNITY_BUILD

nodist_libSwaptionEngines_la_SOURCES = unity.cpp

include ${top_srcdir}/mk/unity.mk

EXTRA_DIST = $(cpp_files)

else

libSwaptionEngines_la_SOURCES = $(cpp_files)

endif

noinst_LTLIBRARIES = libSwaptionEngines.la

include ${top_srcdir}/mk/all.mk

