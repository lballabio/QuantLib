
SUBDIRS = pages Examples images

EXTRA_DIST = \
             makefile.mak \
             quantlib.css \
             quantlib.doxy \
             quantlibfooter.html \
             quantlibfooteronline.html \
             quantlibheader.html \
             quantlibheader.tex \
             README.txt

docs-all: docs-ps docs-pdf
docs-html: docs-html-offline
docs-html-offline:
	$(SED) -e "s/_MAN           = NO/_MAN           = YES/" \
           -e "s|ql_basepath|${BASEPATH}/|" \
	       quantlib.doxy > .quantlib.doxy
	$(DOXYGEN) .quantlib.doxy
	rm .quantlib.doxy
	cp images/*.jpg html
	cp images/*.png html
	cp images/*.eps latex
	cp images/*.pdf latex
docs-html-online:
	$(SED) -e "s/quantlibfooter.html/quantlibfooteronline.html/" \
	       -e "s/_MAN           = NO/_MAN           = YES/" \
           -e "s|ql_basepath|${BASEPATH}/|" \
	       quantlib.doxy > .quantlib.doxy
	$(DOXYGEN) .quantlib.doxy
	rm .quantlib.doxy
	cp images/*.jpg html
	cp images/*.png html6
	cp images/*.eps latex
	cp images/*.pdf latex
docs-pdf: tex-files
	cd latex && $(PDFLATEX) refman
	cd latex && $(MAKEINDEX) refman
	cd latex && $(PDFLATEX) refman
docs-ps: tex-files
	cd latex && $(LATEX) refman
	cd latex && $(MAKEINDEX) refman
	cd latex && $(LATEX) refman
	cd latex && $(DVIPS) -o refman.ps refman
docs-man: docs-html
docs-clean:
	rm -Rf man
	rm -Rf html
	rm -Rf latex

tex-files: docs-html
	cp qlintro.tex latex/
	mv latex/refman.tex .refman.tex
	$(SED) -e "/Page Index/d" \
           -e "/input{pages}/d" \
           -e "/Page Documentation/d" \
           -e "/input{group}/d" \
           -e "/include{history}/d" \
           -e "/include{index}/d" \
           -e "/include{install}/d" \
           -e "/include{license}/d" \
           -e "/include{overview}/d" \
           -e "/include{resources}/d" \
           -e "/include{usage}/d" \
           -e "/include{where}/d" \
           -e "s/ple Documentation}/ple Documentation}\\\\label{exchap}/" \
	       .refman.tex > latex/refman.tex
	rm -f .refman.tex
	mv latex/deprecated.tex .deprecated.tex
	$(SED) -e "s/section/chapter/" .deprecated.tex > latex/deprecated.tex
	rm -f .deprecated.tex
	mv latex/bug.tex .bug.tex
	$(SED) -e "s/section/chapter/" .bug.tex > latex/bug.tex
	rm -f .bug.tex
	mv latex/todo.tex .todo.tex
	$(SED) -e "s/section/chapter/" .todo.tex > latex/todo.tex
	rm -f .todo.tex

