
AM_CPPFLAGS = -I${top_builddir} -I${top_srcdir}

SUBDIRS = \
    BasketLosses \
    BermudanSwaption \
    Bonds \
    CallableBonds \
    CDS \
    ConvertibleBonds \
    CVAIRS \
    DiscreteHedging \
    EquityOption \
    FittedBondCurve \
    FRA \
    Gaussian1dModels \
    GlobalOptimizer \
    LatentModel \
    MarketModels \
    MultidimIntegral \
    Replication \
    Repo \
    Swap

SUBDIR_CHECKS = $(SUBDIRS:%=%.check)

EXTRA_DIST = CMakeLists.txt README.txt

.PHONY: examples check-examples $(SUBDIRS) $(SUBDIR_CHECKS)

examples: $(SUBDIRS)

check-examples: $(SUBDIR_CHECKS)

$(SUBDIRS):
	$(MAKE) -C $@ examples

$(SUBDIR_CHECKS):
	$(MAKE) -C ${@:.check=} check-examples
