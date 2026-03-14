
EXTRA_DIST = \
             quantlib.doxy \
             quantlibextra.css \
             quantlibfooter.html \
             quantlibheader.html

DOXYGEN_CONFIG := quantlib.doxy
DOXYGEN_CUSTOM := quantlibextra.css quantlibheader.html quantlibfooter.html
DOXYGEN_INPUT := $(shell find ${top_srcdir} -name *.hpp) \
                 $(wildcard pages/*.docs)

BASEPATH = $(shell cd @abs_top_srcdir@ && pwd)

.PHONY: docs docs-clean

dist-hook:
	mkdir -p $(distdir)/pages $(distdir)/images
	cp -p $(srcdir)/pages/*.docs $(distdir)/pages
	cp -p $(srcdir)/images/*.jpg \
	      $(srcdir)/images/*.ico \
	      $(distdir)/images

docs: .time-stamp

clean-local:
	rm -Rf html
	rm -f .time-stamp


.time-stamp: $(DOXYGEN_CONFIG) $(DOXYGEN_CUSTOM) $(DOXYGEN_INPUT) ../LICENSE.TXT
	cp -p ${top_srcdir}/LICENSE.TXT ${top_srcdir}/Contributors.txt .
	$(SED) -e "s|ql_basepath|${BASEPATH}/|" \
           -e "s|ql_version|$(VERSION)|" \
	       ${srcdir}/quantlib.doxy > .quantlib.doxy
	$(DOXYGEN) .quantlib.doxy
	rm -f .quantlib.doxy
	rm -f html/*.md5
	rm -f html/*.map
	touch .time-stamp
