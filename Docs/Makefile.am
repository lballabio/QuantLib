
# $Id$
# $Source$
# $Log$
# Revision 1.23  2001/09/11 15:26:33  lballabio
# Finite differences docs enhanced
#
# Revision 1.22  2001/09/04 15:15:28  lballabio
# Finite difference docs updated
#
# Revision 1.21  2001/09/03 13:32:43  lballabio
# *** empty log message ***
#
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
             core.docs \
             findiff.docs \
             mcarlo.docs \
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
	       -e "54,$s/input{index}//" \
	       -e "54,$s/include{install}//" \
	       -e "54,$s/include{license}//" \
	       -e "54,$s/include{platforms}//" \
	       -e "54,$s/include{usage}//" \
	       -e "54,$s/include{where}//" \
	       -e "54,$s/include{todo}//" \
	       -e "54,$s/include{core}//" \
	       -e "54,$s/include{findiff}//" \
	       -e "54,$s/include{mcarlo}//" \
	       oldrefman.tex > latex/refman.tex
	rm -f oldrefman.tex

