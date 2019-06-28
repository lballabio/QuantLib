
AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
	all.hpp \
	bermudanswaptionexercisevalue.hpp \
	collectnodedata.hpp \
	exercisevalue.hpp \
	lsstrategy.hpp \
	marketmodelbasissystem.hpp \
	marketmodelparametricexercise.hpp \
	nodedataprovider.hpp \
	nothingexercisevalue.hpp \
	parametricexerciseadapter.hpp \
	swapbasissystem.hpp \
	swapforwardbasissystem.hpp \
	swapratetrigger.hpp \
	triggeredswapexercise.hpp \
	upperboundengine.hpp

cpp_files = \
	bermudanswaptionexercisevalue.cpp \
	collectnodedata.cpp \
	lsstrategy.cpp \
	nothingexercisevalue.cpp \
	parametricexerciseadapter.cpp \
	swapbasissystem.cpp \
	swapforwardbasissystem.cpp \
	swapratetrigger.cpp \
	triggeredswapexercise.cpp \
	upperboundengine.cpp

if UNITY_BUILD

nodist_libMarketModelsCallability_la_SOURCES = unity.cpp

include ${top_srcdir}/mk/unity.mk

EXTRA_DIST = $(cpp_files)

else

libMarketModelsCallability_la_SOURCES = $(cpp_files)

endif

noinst_LTLIBRARIES = libMarketModelsCallability.la

include ${top_srcdir}/mk/all.mk

