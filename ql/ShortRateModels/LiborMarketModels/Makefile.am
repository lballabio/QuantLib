
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/ShortRateModels/LiborMarketModels

include_HEADERS = \
	all.hpp \
	lfmcovarproxy.hpp \
	lmexpcorrmodel.hpp \
	lmvolmodel.hpp \
	liborforwardmodel.hpp  \
	lmfixedvolmodel.hpp \
	lmcorrmodel.hpp \
	lmlinexpvolmodel.hpp

libLiborMarketModels_la_SOURCES = \
	lfmcovarproxy.cpp \
	lmexpcorrmodel.cpp \
	lmvolmodel.cpp \
	liborforwardmodel.cpp \
	lmfixedvolmodel.cpp \
	lmcorrmodel.cpp \
	lmlinexpvolmodel.cpp


noinst_LTLIBRARIES = libLiborMarketModels.la

EXTRA_DIST = makefile.mak

