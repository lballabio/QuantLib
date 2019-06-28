
AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
    all.hpp \
	haganirregularswaptionengine.hpp \
	irregularswap.hpp \
	irregularswaption.hpp

cpp_files = \
	haganirregularswaptionengine.cpp \
	irregularswap.cpp \
	irregularswaption.cpp

if UNITY_BUILD

nodist_libSwaptions_la_SOURCES = unity.cpp

include ${top_srcdir}/mk/unity.mk

EXTRA_DIST = $(cpp_files)

else

libSwaptions_la_SOURCES = $(cpp_files)

endif

noinst_LTLIBRARIES = libSwaptions.la

include ${top_srcdir}/mk/all.mk

