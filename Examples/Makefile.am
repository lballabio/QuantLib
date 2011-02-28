
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}

SUBDIRS = \
    BermudanSwaption \
    Bonds \
    CallableBonds \
    CDS \
    ConvertibleBonds \
    DiscreteHedging \
    EquityOption \
    FittedBondCurve \
    FRA \
    MarketModels \
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
