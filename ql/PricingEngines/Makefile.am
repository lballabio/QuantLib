
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}
includedir = $(prefix)/include/ql/PricingEngines

SUBDIRS = Asian Barrier Cliquet Forward Quanto Vanilla

include_HEADERS = \
    all.hpp \
    americanpayoffatexpiry.hpp \
    americanpayoffathit.hpp \
    blackformula.hpp \
    core.hpp \
    genericmodelengine.hpp \
    latticeshortratemodelengine.hpp \
    mcsimulation.hpp

EXTRA_DIST=makefile.mak
