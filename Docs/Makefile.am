
# $Id$
# $Source$
# $Log$
# Revision 1.7  2001/05/24 12:52:01  nando
# smoothing #include xx.hpp
#


EXTRA_DIST = README.txt quantlib.doxy quantlibfooter.html makefile.mak

docs-html:
	$(DOXYGEN) quantlib.doxy
docs-ps: docs-html
	$(MAKE) -C latex ps
docs-pdf: docs-html
	$(MAKE) -C latex pdf
docs-man: docs-html

