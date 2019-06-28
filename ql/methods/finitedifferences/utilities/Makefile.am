
AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
	all.hpp \
	bsmrndcalculator.hpp \
    cevrndcalculator.hpp \
	fdmaffinemodeltermstructure.hpp \
	fdmaffinemodelswapinnervalue.hpp \
	fdmboundaryconditionset.hpp \
	fdmdirichletboundary.hpp \
	fdmdiscountdirichletboundary.hpp \
	fdmdividendhandler.hpp \
	fdmindicesonboundary.hpp \
	fdminnervaluecalculator.hpp \
	fdmmesherintegral.hpp \
	fdmquantohelper.hpp \
	fdmtimedepdirichletboundary.hpp \
	gbsmrndcalculator.hpp \
	hestonrndcalculator.hpp \
	localvolrndcalculator.hpp \
	riskneutraldensitycalculator.hpp \
	squarerootprocessrndcalculator.hpp

cpp_files = \
    bsmrndcalculator.cpp \
    cevrndcalculator.cpp \
	fdmaffinemodeltermstructure.cpp \
	fdmaffinemodelswapinnervalue.cpp \
	fdmdirichletboundary.cpp \
	fdmdiscountdirichletboundary.cpp \
	fdmdividendhandler.cpp \
	fdmindicesonboundary.cpp \
	fdminnervaluecalculator.cpp \
	fdmmesherintegral.cpp \
	fdmquantohelper.cpp \
	fdmtimedepdirichletboundary.cpp \
	gbsmrndcalculator.cpp \
	hestonrndcalculator.cpp \
	localvolrndcalculator.cpp \
	riskneutraldensitycalculator.cpp \
	squarerootprocessrndcalculator.cpp

if UNITY_BUILD

nodist_libFdmUtils_la_SOURCES = unity.cpp

include ${top_srcdir}/mk/unity.mk

EXTRA_DIST = $(cpp_files)

else

libFdmUtils_la_SOURCES = $(cpp_files)

endif

noinst_LTLIBRARIES = libFdmUtils.la

include ${top_srcdir}/mk/all.mk

