
# $Id$

EXTRA_DIST = \
             authors.docs \
             calendars.docs \
             cashflows.docs \
             coreclasses.docs \
             currencies.docs \
             daycounters.docs \
             examples.docs \
             findiff.docs \
             groups.docs \
             history.docs \
             index.docs \
             indexes.docs \
             install.docs \
             instruments.docs \
             license.docs \
             makefile.mak \
             math.docs \
             mcarlo.docs \
             overview.docs \
             patterns.docs \
             platforms.docs \
             pricers.docs \
             quantlib.css \
             quantlib.doxy \
             quantlibfooter.html \
             quantlibfooteronline.html \
             quantlibheader.html \
             quantlibheader.tex \
             randomnumbers.docs \
             README.txt \
             resources.docs \
             solvers1d.docs \
             termstructures.docs \
             usage.docs \
             utilities.docs \
             where.docs

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
	mv latex/refman.tex oldrefman.tex
	$(SED) -e "/Page Index/d" \
	       -e "/input{pages}/d" \
	       -e "/Page Documentation/d" \
	       -e "76,$$ s/input{index}/ /" \
	       -e "76,$$ s/include{index}/ /" \
	       -e "76,$$ s/input{overview}/ /" \
	       -e "76,$$ s/include{overview}/ /" \
	       -e "76,$$ s/input{where}/ /" \
	       -e "76,$$ s/include{where}/ /" \
	       -e "76,$$ s/input{install}/ /" \
	       -e "76,$$ s/include{install}/ /" \
	       -e "76,$$ s/input{usage}/ /" \
	       -e "76,$$ s/include{usage}/ /" \
	       -e "76,$$ s/input{platforms}/ /" \
	       -e "76,$$ s/include{platforms}/ /" \
	       -e "76,$$ s/input{history}/ /" \
	       -e "76,$$ s/include{history}/ /" \
	       -e "76,$$ s/input{todo}/ /" \
	       -e "76,$$ s/include{todo}/ /" \
	       -e "76,$$ s/input{resources}/ /" \
	       -e "76,$$ s/include{resources}/ /" \
	       -e "76,$$ s/input{group}/ /" \
	       -e "76,$$ s/include{group}/ /" \
	       -e "76,$$ s/input{license}/ /" \
	       -e "76,$$ s/include{license}/ /" \
	       -e "76,$$ s/input{coreclasses}/ /" \
	       -e "76,$$ s/include{coreclasses}/ /" \
	       -e "76,$$ s/input{calendars}/ /" \
	       -e "76,$$ s/include{calendars}/ /" \
	       -e "76,$$ s/input{cashflows}/ /" \
	       -e "76,$$ s/include{cashflows}/ /" \
	       -e "76,$$ s/input{currencies}/ /" \
	       -e "76,$$ s/include{currencies}/ /" \
	       -e "76,$$ s/input{daycounters}/ /" \
	       -e "76,$$ s/include{daycounters}/ /" \
	       -e "76,$$ s/input{findiff}/ /" \
	       -e "76,$$ s/include{findiff}/ /" \
	       -e "76,$$ s/input{indexes}/ /" \
	       -e "76,$$ s/include{indexes}/ /" \
	       -e "76,$$ s/input{instruments}/ /" \
	       -e "76,$$ s/include{instruments}/ /" \
	       -e "76,$$ s/input{math}/ /" \
	       -e "76,$$ s/include{math}/ /" \
	       -e "76,$$ s/input{mcarlo}/ /" \
	       -e "76,$$ s/include{mcarlo}/ /" \
	       -e "76,$$ s/input{patterns}/ /" \
	       -e "76,$$ s/include{patterns}/ /" \
	       -e "76,$$ s/input{pricers}/ /" \
	       -e "76,$$ s/include{pricers}/ /" \
	       -e "76,$$ s/input{randomnumbers}/ /" \
	       -e "76,$$ s/include{randomnumbers}/ /" \
	       -e "76,$$ s/input{solvers1d}/ /" \
	       -e "76,$$ s/include{solvers1d}/ /" \
	       -e "76,$$ s/input{termstructures}/ /" \
	       -e "76,$$ s/include{termstructures}/ /" \
	       -e "76,$$ s/input{utilities}/ /" \
	       -e "76,$$ s/include{utilities}/ /" \
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


