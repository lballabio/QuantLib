
SUBDIRS = config ql m4 man Docs Examples test-suite

ACLOCAL_AMFLAGS = -I m4

bin_SCRIPTS = quantlib-config

if EMACS_FOUND
dist_lisp_LISP = quantlib.el
endif

m4datadir = $(datadir)/aclocal
dist_m4data_DATA = quantlib.m4

EXTRA_DIST = \
	Announce.txt \
	Authors.txt \
	Bugs.txt \
	ChangeLog.txt \
	Contributors.txt \
	LICENSE.TXT \
	News.txt \
	QuantLib.dev \
	QuantLib_vc7.sln \
	QuantLib_vc7.vcproj \
	QuantLib_vc8.sln \
	QuantLib_vc8.vcproj \
	QuantLib_vc9.sln \
	QuantLib_vc9.vcproj \
	Readme.txt \
	autogen.sh

.PHONY: examples check-examples
examples:
	$(MAKE) -C Examples examples
check-examples:
	$(MAKE) -C Examples check-examples

.PHONY: benchmark
benchmark:
	$(MAKE) -C test-suite benchmark

.PHONY: docs docs-man docs-online docs-dist docs-clean
BASEPATH = @abs_top_srcdir@
docs:
	BASEPATH=$(BASEPATH) $(MAKE) -C Docs docs-all
html-local:
	BASEPATH=$(BASEPATH) $(MAKE) -C Docs docs-html
docs-man:
	BASEPATH=$(BASEPATH) $(MAKE) -C Docs docs-man
docs-online:
	BASEPATH=$(BASEPATH) $(MAKE) -C Docs docs-online
docs-dist:
	BASEPATH=$(BASEPATH) $(MAKE) -C Docs docs-dist
docs-clean:
	$(MAKE) -C Docs docs-clean

dist-hook:
	mkdir -p $(distdir)/build
