
# $Id$
# $Source$
# $Log$
# Revision 1.20  2001/08/27 11:56:25  nando
# improved installer
#
# Revision 1.19  2001/08/23 11:22:17  nando
# Win32 documentation installer
#
# Revision 1.18  2001/08/23 08:42:57  lballabio
# A few fixes for making docs on Linux
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

docs-all: docs-pdf
docs-html:
	$(DOXYGEN) quantlib.linux.doxy
	cp images/*.jpg html
	cp images/*.eps latex
	cp images/*.pdf latex
docs-ps: tex-files
	$(MAKE) -C latex ps
docs-pdf: tex-files
	$(MAKE) -C latex pdf
docs-man: docs-html
docs-clean:
	rm -Rf man
	rm -Rf html
	rm -Rf latex

tex-files: docs-html
	mv latex/refman.tex oldrefman.tex
	$(SED) -e "/Page Index/d" \
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

