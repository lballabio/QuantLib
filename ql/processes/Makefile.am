
AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
	all.hpp \
	batesprocess.hpp \
	blackscholesprocess.hpp \
	endeulerdiscretization.hpp \
	eulerdiscretization.hpp \
	forwardmeasureprocess.hpp \
	g2process.hpp \
	geometricbrownianprocess.hpp \
	gjrgarchprocess.hpp \
	gsrprocess.hpp \
	gsrprocesscore.hpp \
	hestonprocess.hpp \
	hullwhiteprocess.hpp \
	hybridhestonhullwhiteprocess.hpp \
	jointstochasticprocess.hpp \
	merton76process.hpp \
	mfstateprocess.hpp \
	ornsteinuhlenbeckprocess.hpp \
	squarerootprocess.hpp \
	stochasticprocessarray.hpp

cpp_files = \
	batesprocess.cpp \
	blackscholesprocess.cpp \
	endeulerdiscretization.cpp \
	eulerdiscretization.cpp \
	forwardmeasureprocess.cpp \
	g2process.cpp \
	geometricbrownianprocess.cpp \
	gjrgarchprocess.cpp \
	gsrprocess.cpp \
	gsrprocesscore.cpp \
	hestonprocess.cpp \
	hullwhiteprocess.cpp \
	hybridhestonhullwhiteprocess.cpp \
	jointstochasticprocess.cpp \
	merton76process.cpp \
	mfstateprocess.cpp \
	ornsteinuhlenbeckprocess.cpp \
	squarerootprocess.cpp \
	stochasticprocessarray.cpp

if UNITY_BUILD

nodist_libProcesses_la_SOURCES = unity.cpp

include ${top_srcdir}/mk/unity.mk

EXTRA_DIST = $(cpp_files)

else

libProcesses_la_SOURCES = $(cpp_files)

endif

noinst_LTLIBRARIES = libProcesses.la

include ${top_srcdir}/mk/all.mk

