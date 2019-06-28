
AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
	all.hpp \
	lfmcovarparam.hpp \
	lfmcovarproxy.hpp \
	lfmhullwhiteparam.hpp \
	lfmprocess.hpp \
	lfmswaptionengine.hpp \
	liborforwardmodel.hpp \
	lmconstwrappercorrmodel.hpp \
	lmconstwrappervolmodel.hpp \
	lmcorrmodel.hpp \
	lmexpcorrmodel.hpp \
	lmextlinexpvolmodel.hpp \
	lmfixedvolmodel.hpp \
	lmlinexpcorrmodel.hpp \
	lmlinexpvolmodel.hpp \
	lmvolmodel.hpp

cpp_files = \
	lfmcovarparam.cpp \
	lfmcovarproxy.cpp \
	lfmhullwhiteparam.cpp \
	lfmprocess.cpp \
	lfmswaptionengine.cpp \
	liborforwardmodel.cpp \
	lmcorrmodel.cpp \
	lmexpcorrmodel.cpp \
	lmextlinexpvolmodel.cpp \
	lmfixedvolmodel.cpp \
	lmlinexpcorrmodel.cpp \
	lmlinexpvolmodel.cpp \
	lmvolmodel.cpp

if UNITY_BUILD

nodist_libLiborMarketModels_la_SOURCES = unity.cpp

include ${top_srcdir}/mk/unity.mk

EXTRA_DIST = $(cpp_files)

else

libLiborMarketModels_la_SOURCES = $(cpp_files)

endif

noinst_LTLIBRARIES = libLiborMarketModels.la

include ${top_srcdir}/mk/all.mk

