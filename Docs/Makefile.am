
# $Id$

SUBDIRS = pages

EXTRA_DIST = \
             makefile.mak \
             quantlib.css \
             quantlib.doxy \
             quantlibfooter.html \
             quantlibfooteronline.html \
             quantlibheader.html \
             quantlibheader.tex \
             userman.tex \
             README.txt

docs-all: docs-pdf docs-ps
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
	extra_mem_top=263000 max_strings=30000 $(MAKE) -C latex ps
docs-pdf: tex-files pdf-figures
	extra_mem_top=263000 max_strings=30000 $(MAKE) -C latex pdf
docs-man: docs-html
docs-clean:
	rm -Rf man
	rm -Rf html
	rm -Rf latex

tex-files: docs-html
	cp userman.tex latex/userman.tex
	mv latex/refman.tex oldrefman.tex
	$(SED) -e "/Page Index/d" \
	       -e "/input{pages}/d" \
	       -e "/Page Documentation/d" \
	       -e "s/input{index}/ /" \
	       -e "s/include{index}/ /" \
	       -e "s/input{overview}/ /" \
	       -e "s/include{overview}/ /" \
	       -e "s/input{where}/ /" \
	       -e "s/include{where}/ /" \
	       -e "s/input{install}/ /" \
	       -e "s/include{install}/ /" \
	       -e "s/input{usage}/ /" \
	       -e "s/include{usage}/ /" \
	       -e "s/input{platforms}/ /" \
	       -e "s/include{platforms}/ /" \
	       -e "s/input{history}/ /" \
	       -e "s/include{history}/ /" \
	       -e "s/input{todo}/ /" \
	       -e "s/include{todo}/ /" \
	       -e "s/input{resources}/ /" \
	       -e "s/include{resources}/ /" \
	       -e "s/input{group}/ /" \
	       -e "s/include{group}/ /" \
	       -e "s/input{license}/ /" \
	       -e "s/include{license}/ /" \
	       -e "s/input{coreclasses}/ /" \
	       -e "s/include{coreclasses}/ /" \
	       -e "s/input{cashflows}/ /" \
	       -e "s/include{cashflows}/ /" \
	       -e "s/input{currencies}/ /" \
	       -e "s/include{currencies}/ /" \
	       -e "s/input{datetime}/ /" \
	       -e "s/include{datetime}/ /" \
	       -e "s/input{findiff}/ /" \
	       -e "s/include{findiff}/ /" \
	       -e "s/input{indexes}/ /" \
	       -e "s/include{indexes}/ /" \
	       -e "s/input{instruments}/ /" \
	       -e "s/include{instruments}/ /" \
	       -e "s/input{math}/ /" \
	       -e "s/include{math}/ /" \
	       -e "s/input{mcarlo}/ /" \
	       -e "s/include{mcarlo}/ /" \
	       -e "s/input{patterns}/ /" \
	       -e "s/include{patterns}/ /" \
	       -e "s/input{pricers}/ /" \
	       -e "s/include{pricers}/ /" \
	       -e "s/input{randomnumbers}/ /" \
	       -e "s/include{randomnumbers}/ /" \
	       -e "s/input{solvers1d}/ /" \
	       -e "s/include{solvers1d}/ /" \
	       -e "s/input{termstructures}/ /" \
	       -e "s/include{termstructures}/ /" \
	       -e "s/input{utilities}/ /" \
	       -e "s/include{utilities}/ /" \
	       -e "s/ple Documentation}/ple Documentation}\\\\label{exchap}/" \
	       oldrefman.tex > latex/refman.tex
	rm -f oldrefman.tex

# some uncharted combination of dot and epstopdf leads to cropped pdf figures.
# removing the %%BoundingBox line and rerunning epstopdf fixes them.
pdf-figures: docs-html
	for i in latex/a0*.eps ; do \
		mv $$i $$i.bak ; \
		sed -e "/%%BoundingBox/d" $$i.bak > $$i ; \
		epstopdf $$i 2> /dev/null ; \
                mv $$i.bak $$i ; \
	done


