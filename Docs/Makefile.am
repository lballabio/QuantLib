
# $Id$
# $Source$
# $Log$
# Revision 1.11  2001/06/25 10:04:01  nando
# R019-branch-merge5 merged into trunk
#

SUBDIRS=Examples

EXTRA_DIST = makefile.mak \
             quantlib.doxy \
             quantlib.linux.doxy \
             quantlib.win32.doxy \
             quantlibfooter.html \
             README.txt

docs-html:
	$(DOXYGEN) quantlib.linux.doxy
docs-ps: docs-html
	$(MAKE) -C latex ps
docs-pdf: docs-html
	$(MAKE) -C latex pdf
docs-man: docs-html
docs-clean:
	rm -Rf html latex man

