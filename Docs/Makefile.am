
# $Id$
# $Source$
# $Log$
# Revision 1.8  2001/06/05 09:35:13  lballabio
# Updated docs to use Doxygen 1.2.8
#
# Revision 1.7  2001/05/24 12:52:01  nando
# smoothing #include xx.hpp
#


EXTRA_DIST = README.txt \
             quantlib.doxy quantlib.linux.doxy quantlib.win32.doxy \
             quantlibfooter.html \
             makefile.mak

docs-html:
	$(DOXYGEN) quantlib.linux.doxy
docs-ps: docs-html
	$(MAKE) -C latex ps
docs-pdf: docs-html
	$(MAKE) -C latex pdf
docs-man: docs-html
docs-clean:
    rm -Rf html latex man
