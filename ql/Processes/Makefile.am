
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/Processes

include_HEADERS = \
	all.hpp \
	blackscholesprocess.hpp \
	eulerdiscretization.hpp \
	forwardmeasureprocess.hpp \
	g2process.hpp \
	geometricbrownianprocess.hpp \
	hestonprocess.hpp \
	hullwhiteprocess.hpp \
	lfmcovarparam.hpp \
	lfmhullwhiteparam.hpp \
	lfmprocess.hpp \
	merton76process.hpp \
	ornsteinuhlenbeckprocess.hpp \
	squarerootprocess.hpp \
	stochasticprocessarray.hpp

libProcesses_la_SOURCES = \
	blackscholesprocess.cpp \
	eulerdiscretization.cpp \
	forwardmeasureprocess.cpp \
	g2process.cpp \
	geometricbrownianprocess.cpp \
	hestonprocess.cpp \
	hullwhiteprocess.cpp \
	lfmcovarparam.cpp \
	lfmhullwhiteparam.cpp \
	lfmprocess.cpp \
	merton76process.cpp \
	ornsteinuhlenbeckprocess.cpp \
	squarerootprocess.cpp \
	stochasticprocessarray.cpp

noinst_LTLIBRARIES = libProcesses.la

EXTRA_DIST = makefile.mak

