
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

.PHONY: docs-all docs-html docs-clean
.PHONY: docs-dist

dist-hook:
	mkdir -p $(distdir)/pages $(distdir)/images
	cp -p $(srcdir)/pages/*.docs $(distdir)/pages
	cp -p $(srcdir)/images/*.jpg \
	      $(srcdir)/images/*.ico \
	      $(distdir)/images

docs-all: docs-html
docs-html: .time-stamp

docs-dist: docs-html
	ln -s html QuantLib-docs-$(VERSION)-html
	tar chf - QuantLib-docs-$(VERSION)-html \
	    | GZIP=--best gzip -c > QuantLib-docs-$(VERSION)-html.tar.gz
	rm QuantLib-docs-$(VERSION)-html

docs-clean:
	rm -Rf html
	rm -f .time-stamp*


.time-stamp: $(DOXYGEN_CONFIG) $(DOXYGEN_CUSTOM) $(DOXYGEN_INPUT) ../LICENSE.TXT
	cp -p ../LICENSE.TXT .
	$(SED) -e "s|ql_basepath|${BASEPATH}/|" \
           -e "s|ql_version|$(VERSION)|" \
	       quantlib.doxy > .quantlib.doxy
	$(DOXYGEN) .quantlib.doxy
	rm -f .quantlib.doxy
	rm -f html/*.md5
	rm -f html/*.map
	touch .time-stamp
