
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/PricingEngines

SUBDIRS = Barrier Cliquet Forward Quanto Vanilla

include_HEADERS = \
    genericmodelengine.hpp \
    latticeshortratemodelengine.hpp \
    mcsimulation.hpp

EXTRA_DIST=makefile.mak
