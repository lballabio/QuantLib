
# $Id$

EXTRA_DIST = makefile.mak \
             groups.docs \
             index.docs \
             overview.docs \
             where.docs \
             install.docs \
             usage.docs \
             platforms.docs \
             history.docs \
             resources.docs \
             authors.docs \
             license.docs \
             coreclasses.docs \
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
	       -e "/input{hierarchy}/d" \
	       -e "/Hierarchical Index/d" \
	       -e "64,$$ s/input{index}/ /" \
	       -e "64,$$ s/include{index}/ /" \
	       -e "64,$$ s/input{overview}/ /" \
	       -e "64,$$ s/include{overview}/ /" \
	       -e "64,$$ s/input{where}/ /" \
	       -e "64,$$ s/include{where}/ /" \
	       -e "64,$$ s/input{install}/ /" \
	       -e "64,$$ s/include{install}/ /" \
	       -e "64,$$ s/input{usage}/ /" \
	       -e "64,$$ s/include{usage}/ /" \
	       -e "64,$$ s/input{platforms}/ /" \
	       -e "64,$$ s/include{platforms}/ /" \
	       -e "64,$$ s/input{history}/ /" \
	       -e "64,$$ s/include{history}/ /" \
	       -e "64,$$ s/input{todo}/ /" \
	       -e "64,$$ s/include{todo}/ /" \
	       -e "64,$$ s/input{resources}/ /" \
	       -e "64,$$ s/include{resources}/ /" \
	       -e "64,$$ s/input{group}/ /" \
	       -e "64,$$ s/include{group}/ /" \
	       -e "64,$$ s/input{license}/ /" \
	       -e "64,$$ s/include{license}/ /" \
	       -e "64,$$ s/input{coreclasses}/ /" \
	       -e "64,$$ s/include{coreclasses}/ /" \
	       -e "64,$$ s/input{findiff}/ /" \
	       -e "64,$$ s/include{findiff}/ /" \
	       -e "64,$$ s/input{mcarlo}/ /" \
	       -e "64,$$ s/include{mcarlo}/ /" \
	       -e "s/ple Documentation}/ple Documentation}\\\\label{exchap}/" \
	       oldrefman.tex > latex/refman.tex
	rm -f oldrefman.tex

