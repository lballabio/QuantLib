
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
             examples.docs \
             quantlib.doxy \
             quantlibheader.html \
             quantlibfooter.html \
             quantlibfooteronline.html \
             quantlibheader.tex \
             README.txt

docs-all: docs-pdf
docs-html: docs-html-offline
docs-html-offline:
	$(SED) -e "s/_MAN           = NO/_MAN           = YES/" \
	       quantlib.doxy > quantlib.doxy.temp
	$(DOXYGEN) quantlib.doxy.temp
	rm quantlib.doxy.temp
	cp images/*.jpg html
	cp images/*.png html
	cp images/*.eps latex
	cp images/*.pdf latex
docs-html-online:
	$(SED) -e "s/quantlibfooter.html/quantlibfooteronline.html/" \
	       -e "s/_MAN           = NO/_MAN           = YES/" \
	       quantlib.doxy > quantlib.doxy.temp
	$(DOXYGEN) quantlib.doxy.temp
	rm quantlib.doxy.temp
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
	       -e "60,$$ s/input{index}/ /" \
	       -e "60,$$ s/include{index}/ /" \
	       -e "60,$$ s/input{install}/ /" \
	       -e "60,$$ s/include{install}/ /" \
	       -e "60,$$ s/input{license}/ /" \
	       -e "60,$$ s/include{license}/ /" \
	       -e "60,$$ s/input{platforms}/ /" \
	       -e "60,$$ s/include{platforms}/ /" \
	       -e "60,$$ s/input{usage}/ /" \
	       -e "60,$$ s/include{usage}/ /" \
	       -e "60,$$ s/input{where}/ /" \
	       -e "60,$$ s/include{where}/ /" \
	       -e "60,$$ s/input{todo}/ /" \
	       -e "60,$$ s/include{todo}/ /" \
	       -e "60,$$ s/input{core}/ /" \
	       -e "60,$$ s/include{core}/ /" \
	       -e "60,$$ s/input{findiff}/ /" \
	       -e "60,$$ s/include{findiff}/ /" \
	       -e "60,$$ s/input{mcarlo}/ /" \
	       -e "60,$$ s/include{mcarlo}/ /" \
	       -e "s/ple Documentation}/ple Documentation}\\label{exchap}/" \
	       oldrefman.tex > latex/refman.tex
	rm -f oldrefman.tex

