
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}

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
    LatentModel \
    MarketModels \
    MultidimIntegral \
    Replication \
    Repo \
    Swap

SUBDIR_CHECKS = $(SUBDIRS:%=%.check)

EXTRA_DIST = README.txt

%.check:
	$(MAKE) -C $* check-examples

.PHONY: examples check-examples $(SUBDIRS)

examples: $(SUBDIRS)

check-examples: $(SUBDIR_CHECKS)

$(SUBDIRS):
	$(MAKE) -C $@ examples
