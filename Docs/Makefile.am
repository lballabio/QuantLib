
# $Id$

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
             quantlibheader.html \
             quantlibfooter.html \
             quantlibfooteronline.html \
             quantlibheader.tex \
             README.txt

docs-all: docs-pdf
docs-html: docs-html-offline
docs-html-offline:
	export GENERATE_MAN=YES
	export HTML=
	$(DOXYGEN) quantlib.doxy
	cp images/*.jpg html
	cp images/*.png html
	cp images/*.eps latex
	cp images/*.pdf latex
docs-html-online:
	export GENERATE_MAN=YES
	export HTML=online
	$(DOXYGEN) quantlib.doxy
	cp images/*.jpg html
	cp images/*.png html
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

