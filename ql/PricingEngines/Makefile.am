
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/PricingEngines

SUBDIRS = Asian Barrier Cliquet Forward Lookback Quanto Vanilla

include_HEADERS = \
    genericmodelengine.hpp \
    latticeshortratemodelengine.hpp \
    mcsimulation.hpp

EXTRA_DIST=makefile.mak
