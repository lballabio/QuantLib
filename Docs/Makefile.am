
# $Id$
# $Source$
# $Log$
# Revision 1.17  2001/08/22 17:54:35  lballabio
# Documentation revamped
#
# Revision 1.16  2001/07/26 08:27:00  nando
# wrong path bug fixed
#
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
             groups.docs \
             index.docs \
             install.docs \
             license.docs \
             platforms.docs \
             usage.docs \
             where.docs \
             quantlib.doxy \
             quantlib.linux.doxy \
             quantlib.win32.doxy \
             quantlibheader.html \
             quantlibfooter.html \
             quantlibheader.tex \
             README.txt

docs-html:
	$(DOXYGEN) quantlib.linux.doxy
	cp images/*.jpg html
	cp images/*.eps latex
	cp images/*.pdf latex
docs-ps: tex-files
	$(MAKE) -C latex ps
docs-pdf: tex-files
	mv latex/refman.tex oldrefman.tex
	sed $(SED_CMDS) oldrefman.tex > latex/refman.tex
	rm -f oldrefman.tex
	$(MAKE) -C latex pdf
docs-man: docs-html
docs-clean:
	rm -Rf man
	rm -Rf html
	rm -Rf latex

tex-files: docs-html
	mv latex/refman.tex oldrefman.tex
	sed -e "/Page Index/d" \
        -e "/input{pages}/d" \
        -e "/Page Documentation/d" \
        -e "/input{index}/d" \
        -e "/include{install}/d" \
        -e "/include{license}/d" \
        -e "/include{platforms}/d" \
        -e "/include{usage}/d" \
        -e "/include{where}/d" \
        -e "/include{todo}/d" \
        -e "s/i_ndex/index/" \
        -e "s/w_here/where/" \
        -e "s/i_nstall/install/" \
        -e "s/u_sage/usage/" \
        -e "s/p_latforms/platforms/" \
        -e "s/t_odo/todo/" \
        -e "s/l_icense/license/" \
        oldrefman.tex > latex/refman.tex
	rm -f oldrefman.tex
