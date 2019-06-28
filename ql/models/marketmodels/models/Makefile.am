
AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

this_includedir=${includedir}/${subdir}

this_include_HEADERS = \
	all.hpp \
	abcdvol.hpp \
	alphafinder.hpp \
	alphaform.hpp \
	alphaformconcrete.hpp \
	capletcoterminalalphacalibration.hpp \
	capletcoterminalmaxhomogeneity.hpp \
	capletcoterminalperiodic.hpp \
	capletcoterminalswaptioncalibration.hpp \
	cotswaptofwdadapter.hpp \
	ctsmmcapletcalibration.hpp \
	flatvol.hpp \
	fwdperiodadapter.hpp \
	fwdtocotswapadapter.hpp \
	piecewiseconstantabcdvariance.hpp \
	piecewiseconstantvariance.hpp \
	pseudorootfacade.hpp \
	volatilityinterpolationspecifier.hpp \
	volatilityinterpolationspecifierabcd.hpp

cpp_files = \
	abcdvol.cpp \
	alphafinder.cpp \
	alphaformconcrete.cpp \
	capletcoterminalalphacalibration.cpp \
	capletcoterminalmaxhomogeneity.cpp \
	capletcoterminalperiodic.cpp \
	capletcoterminalswaptioncalibration.cpp \
	cotswaptofwdadapter.cpp \
	ctsmmcapletcalibration.cpp \
	flatvol.cpp \
	fwdperiodadapter.cpp \
	fwdtocotswapadapter.cpp \
	piecewiseconstantabcdvariance.cpp \
	piecewiseconstantvariance.cpp \
	pseudorootfacade.cpp \
	volatilityinterpolationspecifier.cpp \
	volatilityinterpolationspecifierabcd.cpp

if UNITY_BUILD

nodist_libMarketModelsModels_la_SOURCES = unity.cpp

include ${top_srcdir}/mk/unity.mk

EXTRA_DIST = $(cpp_files)

else

libMarketModelsModels_la_SOURCES = $(cpp_files)

endif

noinst_LTLIBRARIES = libMarketModelsModels.la

include ${top_srcdir}/mk/all.mk

