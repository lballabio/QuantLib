
EXTRA_DIST = \
             quantlib.css \
             quantlib.doxy \
             quantlibfooter.html \
             quantlibfooteronline.html \
             quantlibheader.html \
             qlintro.tex \
             quantlibheader.tex \
             makefile.mak \
             README.txt

DOXYGEN_CONFIG := quantlib.doxy
DOXYGEN_CUSTOM := qlintro.tex quantlibheader.tex \
                  quantlib.css quantlibheader.html quantlibfooter.html
DOXYGEN_INPUT := $(shell find ${top_srcdir} -name *.hpp) \
                 $(wildcard pages/*.docs)

%.gz: %
	rm -f $@
	gzip --best $<
%.tar: %
	tar chf $@ $<
	rm $<

.PHONY: docs-all docs-ps docs-pdf docs-html docs-man docs-online docs-clean
.PHONY: docs-dist docs-ps-dist docs-pdf-dist docs-html-dist docs-man-dist

dist-hook:
	mkdir -p $(distdir)/pages $(distdir)/Examples $(distdir)/images
	cp -p $(srcdir)/pages/*.docs $(distdir)/pages
	cp -p $(srcdir)/Examples/*.cpp $(distdir)/Examples
	cp -p $(srcdir)/images/*.bmp $(srcdir)/images/*.eps \
	      $(srcdir)/images/*.jpg $(srcdir)/images/*.png \
	      $(srcdir)/images/*.pdf $(srcdir)/images/*.ico $(distdir)/images

docs-all: docs-html docs-ps docs-pdf docs-man
docs-dist: docs-pdf-dist docs-ps-dist docs-html-dist docs-man-dist

docs-html: .time-stamp-html
.time-stamp-html: .time-stamp
	cp images/*.jpg html
	cp images/*.png html
	cp images/*.ico html
	touch .time-stamp-html

docs-html-dist: docs-html
	ln -s html QuantLib-docs-$(VERSION)-html
	tar chf - QuantLib-docs-$(VERSION)-html \
	    | GZIP=--best gzip -c > QuantLib-docs-$(VERSION)-html.tar.gz
	rm QuantLib-docs-$(VERSION)-html

docs-online: .time-stamp-online
.time-stamp-online: $(DOXYGEN_CONFIG) $(DOXYGEN_CUSTOM) $(DOXYGEN_INPUT)
	$(SED) -e "s/quantlibfooter/quantlibfooteronline/" \
           -e "s/GENERATE_HTML          = NO/GENERATE_HTML          = YES/" \
           -e "s/HTML_OUTPUT            = html/HTML_OUTPUT            = html-online/" \
           -e "s|ql_basepath|${BASEPATH}/|" \
           -e "s|ql_version|$(VERSION)|" \
	       quantlib.doxy > .quantlib.doxy
	$(DOXYGEN) .quantlib.doxy
	rm .quantlib.doxy
	cp images/*.jpg html-online
	cp images/*.png html-online
	cp images/*.ico html-online
	touch .time-stamp-online

docs-pdf: .time-stamp-pdf
.time-stamp-pdf: .time-stamp .time-stamp-tex
	cd latex && $(PDFLATEX) refman
	cd latex && $(MAKEINDEX) refman
	cd latex && $(PDFLATEX) refman
	touch .time-stamp-pdf

docs-pdf-dist: docs-pdf
	ln latex/refman.pdf QuantLib-docs-$(VERSION).pdf
	gzip --best --force QuantLib-docs-$(VERSION).pdf

docs-ps: .time-stamp-ps
.time-stamp-ps: .time-stamp .time-stamp-tex
	cd latex && $(LATEX) refman
	cd latex && $(MAKEINDEX) refman
	cd latex && $(LATEX) refman
	cd latex && $(DVIPS) -o refman.ps refman
	touch .time-stamp-ps

docs-ps-dist: docs-ps
	ln latex/refman.ps QuantLib-docs-$(VERSION).ps
	gzip --best --force QuantLib-docs-$(VERSION).ps

docs-man: .time-stamp-man
.time-stamp-man: .time-stamp
	$(SED) -e "s/index/QuantLib/" man/man3/index.3 > man/man3/QuantLib.3
	rm man/man3/index.3
	touch .time-stamp-man

docs-man-dist: docs-man
	ln -s man QuantLib-docs-$(VERSION)-man
	tar chf - QuantLib-docs-$(VERSION)-man \
	    | GZIP=--best gzip -c > QuantLib-docs-$(VERSION)-man.tar.gz
	rm QuantLib-docs-$(VERSION)-man


docs-clean:
	rm -Rf man
	rm -Rf html-online html
	rm -Rf latex
	rm -f .time-stamp*


.time-stamp: $(DOXYGEN_CONFIG) $(DOXYGEN_CUSTOM) $(DOXYGEN_INPUT)
	$(SED) -e "s/GENERATE_MAN           = NO/GENERATE_MAN           = YES/" \
           -e "s/GENERATE_HTML          = NO/GENERATE_HTML          = YES/" \
           -e "s/GENERATE_LATEX         = NO/GENERATE_LATEX         = YES/" \
           -e "s|ql_basepath|${BASEPATH}/|" \
           -e "s|ql_version|$(VERSION)|" \
	       quantlib.doxy > .quantlib.doxy
	$(DOXYGEN) .quantlib.doxy
	rm .quantlib.doxy
	touch .time-stamp

tex-files: .time-stamp-tex
.time-stamp-tex: .time-stamp
	cp qlintro.tex latex
	cp images/*.eps latex
	cp images/*.pdf latex
	mv latex/refman.tex .refman.tex
	$(SED) -e "/Page Index/d" \
           -e "/input{pages}/d" \
           -e "/Page Documentation/d" \
           -e "/include{config}/d" \
           -e "/include{faq}/d" \
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
	mv latex/caveats.tex .caveats.tex
	$(SED) -e "s/section/chapter/" .caveats.tex > latex/caveats.tex
	rm -f .caveats.tex
	mv latex/test.tex .test.tex
	$(SED) -e "s/section/chapter/" .test.tex > latex/test.tex
	rm -f .test.tex
	mv latex/todo.tex .todo.tex
	$(SED) -e "s/section/chapter/" .todo.tex > latex/todo.tex
	rm -f .todo.tex
	touch .time-stamp-tex
