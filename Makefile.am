
SUBDIRS = config ql functions man Docs Examples test-suite

bin_SCRIPTS = quantlib-config

if EMACS_FOUND
dist_lisp_LISP = quantlib.el
endif

m4datadir = $(datadir)/aclocal
dist_m4data_DATA = quantlib.m4

EXTRA_DIST = \
	Announce.txt \
	Authors.txt \
	ChangeLog.txt \
	Contributors.txt \
	LICENSE.TXT \
	News.txt \
	QuantLib.dev \
	QuantLib.dsp \
	QuantLib.dsw \
	QuantLib.nsi \
	QuantLib.sln \
	QuantLib.vcproj \
	QuantLib_vc8.sln \
	QuantLib_vc8.vcproj \
	Readme.txt \
	autogen.sh \
	makefile.mak

.PHONY: examples
examples:
	$(MAKE) -C Examples examples

.PHONY: docs docs-man docs-online docs-dist docs-clean
BASEPATH = @abs_top_srcdir@
docs:
	BASEPATH=$(BASEPATH) $(MAKE) -C Docs docs-all
html-local:
	BASEPATH=$(BASEPATH) $(MAKE) -C Docs docs-html
pdf-local:
	BASEPATH=$(BASEPATH) $(MAKE) -C Docs docs-pdf
ps-local:
	BASEPATH=$(BASEPATH) $(MAKE) -C Docs docs-ps
docs-man:
	BASEPATH=$(BASEPATH) $(MAKE) -C Docs docs-man
docs-online:
	BASEPATH=$(BASEPATH) $(MAKE) -C Docs docs-online
docs-dist:
	BASEPATH=$(BASEPATH) $(MAKE) -C Docs docs-dist
docs-clean:
	$(MAKE) -C Docs docs-clean

