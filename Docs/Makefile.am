
# $Id$
# $Source$
# $Log$
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
docs-ps: docs-html
	$(MAKE) -C latex ps
docs-pdf: docs-html
	$(MAKE) -C latex pdf
docs-man: docs-html
docs-clean:
	rm -Rf man
    rm -f html/*.html
    rm -f html/*.gif
    rm -f html/index.*
    rm -f html/doxygen.css
    rm -f html/formula.repository
    rm -f html/graph_legend.dot
    rm -f latex/*.tex
    rm -f latex/*.aux
    rm -f latex/a*.eps
    rm -f latex/a*.pdf
    rm -f latex/refman.*
    rm -f latex/doxygen.sty
    rm -f latex/Makefile
