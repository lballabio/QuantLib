
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}

this_includedir=${includedir}/${subdir}
this_include_HEADERS = \
	all.hpp \
	capletcoterminalcalibration.hpp \
	correlations.hpp \
	coterminaltoforwardadapter.hpp \
	expcorrabcdvol.hpp \
	expcorrflatvol.hpp \
	forwardtocoterminaladapter.hpp \
	piecewiseconstantabcdvariance.hpp \
	piecewiseconstantvariance.hpp \
	swapfromfracorrelationstructure.hpp \
	timedependantcorrelationstructure.hpp

libMarketModelsModels_la_SOURCES = \
	capletcoterminalcalibration.cpp \
	coterminaltoforwardadapter.cpp \
	correlations.cpp \
	expcorrabcdvol.cpp \
	expcorrflatvol.cpp \
	forwardtocoterminaladapter.cpp \
	piecewiseconstantabcdvariance.cpp \
	swapfromfracorrelationstructure.cpp

noinst_LTLIBRARIES = libMarketModelsModels.la

