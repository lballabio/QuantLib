
# $Id$
# $Source$
# $Log$
# Revision 1.15  2001/07/24 16:59:33  nando
# documentation revised
#
# Revision 1.14  2001/07/23 08:30:05  lballabio
# Fixed indentation
#
# Revision 1.13  2001/07/19 16:40:10  lballabio
# Improved docs a bit
#
# Revision 1.12  2001/06/25 11:22:11  enri
# dist target fixed
#
# Revision 1.11  2001/06/25 10:04:01  nando
# R019-branch-merge5 merged into trunk
#

EXTRA_DIST = makefile.mak \
             quantlib.doxy \
             quantlib.linux.doxy \
             quantlib.win32.doxy \
             quantlibfooter.html \
             README.txt

docs-html:
	$(DOXYGEN) quantlib.linux.doxy
	cp *.jpg html
	cp *.eps latex
	cp *.pdf latex
docs-ps: docs-html
	$(MAKE) -C latex ps
docs-pdf: docs-html
	$(MAKE) -C latex pdf
docs-man: docs-html
docs-clean:
	rm -Rf man
	rm -Rf html
	rm -Rf latex

