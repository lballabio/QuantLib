
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}

SUBDIRS =  \
    AmericanOption \
    BermudanSwaption \
    DiscreteHedging \
    EuropeanOption \
    Swap

EXTRA_DIST = \
    makefile.mak \
    README.txt

.PHONY: examples $(SUBDIRS)
examples: $(SUBDIRS)
$(SUBDIRS):
	$(MAKE) -C $@ examples
