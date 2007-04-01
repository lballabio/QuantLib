
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/marketmodels/models

include_HEADERS = \
	all.hpp \
	capletcoterminalcalibration.hpp \
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
	expcorrabcdvol.cpp \
	expcorrflatvol.cpp \
	forwardtocoterminaladapter.cpp \
	piecewiseconstantabcdvariance.cpp \
	swapfromfracorrelationstructure.cpp

noinst_LTLIBRARIES = libMarketModelsModels.la

