
AM_CPPFLAGS = -I${top_srcdir} -I${top_builddir}

SUBDIRS =  \
    AmericanOption \
    BermudanSwaption \
    DiscreteHedging \
    EuropeanOption \
    Swap

EXTRA_DIST = \
    Examples.dsw \
    Examples.sln \
    makefile.mak \
    README.txt

examples:
	for d in $(SUBDIRS); do $(MAKE) -C $$d examples; done

