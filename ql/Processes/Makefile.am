
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/Processes

include_HEADERS = \
	all.hpp \
	blackscholesprocess.hpp \
	eulerdiscretization.hpp \
	geometricbrownianprocess.hpp \
	merton76process.hpp \
	ornsteinuhlenbeckprocess.hpp \
	squarerootprocess.hpp \
	stochasticprocessarray.hpp

EXTRA_DIST = makefile.mak

libProcesses_la_SOURCES = \
	blackscholesprocess.cpp \
	eulerdiscretization.cpp \
	geometricbrownianprocess.cpp \
	merton76process.cpp \
	ornsteinuhlenbeckprocess.cpp \
	squarerootprocess.cpp \
	stochasticprocessarray.cpp

noinst_LTLIBRARIES = libProcesses.la
