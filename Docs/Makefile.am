
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
	buf_size=100000 pool_size=256000 \
	extra_mem_top=263000 extra_mem_bot=263000 \
	max_strings=30000 hash_extra=10000 $(MAKE) -C latex ps
docs-pdf: tex-files pdf-figures
	buf_size=100000 pool_size=256000 \
	extra_mem_top=263000 extra_mem_bot=263000 \
	max_strings=30000 hash_extra=10000 $(MAKE) -C latex pdf
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
           -e "/input{group}/d" \
           -e "/include{coreclasses}/d" \
           -e "/include{currencies}/d" \
           -e "/include{datetime}/d" \
           -e "/include{findiff}/d" \
           -e "/include{fixedincome}/d" \
           -e "/include{history}/d" \
           -e "/include{index}/d" \
           -e "/include{install}/d" \
           -e "/include{instruments}/d" \
           -e "/include{license}/d" \
           -e "/include{math}/d" \
           -e "/include{mcarlo}/d" \
           -e "/include{optimization}/d" \
           -e "/include{overview}/d" \
           -e "/include{patterns}/d" \
           -e "/include{platforms}/d" \
           -e "/include{resources}/d" \
           -e "/include{termstructures}/d" \
           -e "/include{usage}/d" \
           -e "/include{utilities}/d" \
           -e "/include{where}/d" \
           -e "/include{todo}/d" \
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


