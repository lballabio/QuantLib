
AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
	all.hpp \
	africa.hpp \
	america.hpp \
	asia.hpp \
	crypto.hpp \
	europe.hpp \
	exchangeratemanager.hpp \
	oceania.hpp

cpp_files = \
	africa.cpp \
	america.cpp \
	asia.cpp \
	crypto.cpp \
	europe.cpp \
	exchangeratemanager.cpp \
	oceania.cpp

if UNITY_BUILD

nodist_libCurrencies_la_SOURCES = unity.cpp

include ${top_srcdir}/mk/unity.mk

EXTRA_DIST = $(cpp_files)

else

libCurrencies_la_SOURCES = $(cpp_files)

endif

noinst_LTLIBRARIES = libCurrencies.la

include ${top_srcdir}/mk/all.mk

