
SUBDIRS=config ql man Docs Examples test-suite

bin_SCRIPTS = quantlib-config

if EMACS_FOUND
dist_lisp_LISP = quantlib.el
endif

m4datadir = $(datadir)/aclocal
dist_m4data_DATA = quantlib.m4

EXTRA_DIST = \
	Authors.txt \
	ChangeLog.txt \
	Contributors.txt \
	History.txt \
	INSTALL.txt \
	LICENSE.TXT \
	News.txt \
	QuantLib.dsp \
	QuantLib.dsw \
	QuantLib.mak \
	QuantLib.nsi \
	QuantLib.spec \
	Readme.txt \
	TODO.txt \
	bootstrap \
	makefile.mak

.PHONY: examples
examples:
	$(MAKE) -C Examples examples

.PHONY: docs-all docs-html docs-html-online docs-pdf docs-man docs-clean
BASEPATH = @abs_top_srcdir@
docs-all:
	BASEPATH=$(BASEPATH) $(MAKE) -C Docs docs-all
docs-html:
	BASEPATH=$(BASEPATH) $(MAKE) -C Docs docs-html
docs-html-online:
	BASEPATH=$(BASEPATH) $(MAKE) -C Docs docs-html-online
docs-pdf:
	BASEPATH=$(BASEPATH) $(MAKE) -C Docs docs-pdf
docs-man:
	BASEPATH=$(BASEPATH) $(MAKE) -C Docs docs-man
docs-clean:
	$(MAKE) -C Docs docs-clean


