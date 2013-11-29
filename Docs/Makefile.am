
EXTRA_DIST = \
             quantlib.doxy \
             quantlibextra.css \
             quantlibfooter.html \
             quantlibheader.html \
             README.txt

DOXYGEN_CONFIG := quantlib.doxy
DOXYGEN_CUSTOM := quantlibextra.css quantlibheader.html quantlibfooter.html
DOXYGEN_INPUT := $(shell find ${top_srcdir} -name *.hpp) \
                 $(wildcard pages/*.docs)

BASEPATH = @abs_top_srcdir@

%.gz: %
	rm -f $@
	gzip --best $<
%.tar: %
	tar chf $@ $<
	rm $<

.PHONY: docs-all docs-html docs-man docs-clean
.PHONY: docs-dist docs-html-dist docs-man-dist

dist-hook:
	mkdir -p $(distdir)/pages $(distdir)/Examples $(distdir)/images
	cp -p $(srcdir)/pages/*.docs $(distdir)/pages
	cp -p $(srcdir)/Examples/*.cpp $(distdir)/Examples
	cp -p $(srcdir)/images/*.jpg \
	      $(srcdir)/images/*.ico \
	      $(distdir)/images

docs-all: docs-html docs-man
docs-dist: docs-html-dist docs-man-dist

docs-html: .time-stamp

docs-html-dist: docs-html
	ln -s html QuantLib-docs-$(VERSION)-html
	tar chf - QuantLib-docs-$(VERSION)-html \
	    | GZIP=--best gzip -c > QuantLib-docs-$(VERSION)-html.tar.gz
	rm QuantLib-docs-$(VERSION)-html

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
	rm -Rf html
	rm -f .time-stamp*


.time-stamp: $(DOXYGEN_CONFIG) $(DOXYGEN_CUSTOM) $(DOXYGEN_INPUT)
	$(SED) -e "s|ql_basepath|${BASEPATH}/|" \
           -e "s|ql_version|$(VERSION)|" \
	       quantlib.doxy > .quantlib.doxy
	$(DOXYGEN) .quantlib.doxy
	rm -f .quantlib.doxy
	rm -f html/*.md5
	rm -f html/*.map
	touch .time-stamp
