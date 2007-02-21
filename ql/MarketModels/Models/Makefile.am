
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/MarketModels/Models

include_HEADERS = \
	all.hpp \
	capletcoterminalcalibration.hpp \
	expcorrabcdvol.hpp \
	expcorrflatvol.hpp \
	piecewiseconstantabcdvariance.hpp \
	piecewiseconstantvariance.hpp \
	swapfromfracorrelationstructure.hpp \
	timedependantcorrelationstructure.hpp

libMarketModelsModels_la_SOURCES = \
	capletcoterminalcalibration.cpp \
	expcorrabcdvol.cpp \
	expcorrflatvol.cpp \
	piecewiseconstantabcdvariance.cpp \
	swapfromfracorrelationstructure.cpp

noinst_LTLIBRARIES = libMarketModelsModels.la

