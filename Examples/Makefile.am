
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}

SUBDIRS =  \
    BermudanSwaption \
    DiscreteHedging \
    EquityOption \
    Swap \
    ConvertibleBonds

SUBDIR_CHECKS = $(SUBDIRS:%=%.check)

EXTRA_DIST = \
    makefile.mak \
    README.txt

%.check:
	$(MAKE) -C $* check-examples

.PHONY: examples check-examples $(SUBDIRS)

examples: $(SUBDIRS)

check-examples: $(SUBDIR_CHECKS)

$(SUBDIRS):
	$(MAKE) -C $@ examples
