
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}

SUBDIRS =  \
    BermudanSwaption \
    DiscreteHedging \
    EuropeanOption \
    Swap

EXTRA_DIST = \
    Examples.dsw \
    makefile.mak \
    README.txt

examples:
	for d in $(SUBDIRS); do $(MAKE) -C $$d examples; done

