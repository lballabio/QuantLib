
SUBDIRS = ql m4 man Docs Examples test-suite

ACLOCAL_AMFLAGS = -I m4

bin_SCRIPTS = quantlib-config

if EMACS_FOUND
dist_lisp_LISP = quantlib.el
endif

m4datadir = $(datadir)/aclocal
dist_m4data_DATA = quantlib.m4

pkgconfigdir = $(libdir)/pkgconfig
nodist_pkgconfig_DATA = quantlib.pc

EXTRA_DIST = \
	ChangeLog.txt \
	CMakeLists.txt \
	Contributors.txt \
	LICENSE.TXT \
	News.md \
	QuantLib.natvis \
	QuantLib.props \
	QuantLib.spec \
	QuantLib.sln \
	QuantLib.vcxproj \
	QuantLib.vcxproj.filters \
	README.md \
	autogen.sh

.PHONY: examples check-examples
examples:
	$(MAKE) -C Examples examples
check-examples:
	$(MAKE) -C Examples check-examples

.PHONY: benchmark
benchmark:
	$(MAKE) -C test-suite benchmark

.PHONY: docs docs-online docs-dist docs-clean
docs:
	$(MAKE) -C Docs docs-all
html-local:
	$(MAKE) -C Docs docs-html
docs-dist:
	$(MAKE) -C Docs docs-dist
docs-clean:
	$(MAKE) -C Docs docs-clean

dist-hook:
	mkdir -p $(distdir)/build
	mkdir -p $(distdir)/cmake
	cp -p $(srcdir)/cmake/GenerateHeaders.cmake $(distdir)/cmake
	cp -p $(srcdir)/cmake/Platform.cmake $(distdir)/cmake
	cp -p $(srcdir)/cmake/QuantLibConfig.cmake.in $(distdir)/cmake
