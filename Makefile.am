
SUBDIRS = config ql functions man Docs Examples test-suite

bin_SCRIPTS = quantlib-config

if EMACS_FOUND
dist_lisp_LISP = quantlib.el
endif

m4datadir = $(datadir)/aclocal
dist_m4data_DATA = quantlib.m4

EXTRA_DIST = \
	Authors.txt \
	BUGS.txt \
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
	QuantLib.sln \
	QuantLib.vcproj \
	Readme.txt \
	TODO.txt \
	bootstrap \
	makefile.mak

.PHONY: examples
examples:
	$(MAKE) -C Examples examples

.PHONY: docs-all docs-html docs-ps docs-pdf docs-man docs-clean
.PHONY: docs-dist
BASEPATH = @abs_top_srcdir@
docs-all:
	BASEPATH=$(BASEPATH) $(MAKE) -C Docs docs-all
docs-html:
	BASEPATH=$(BASEPATH) $(MAKE) -C Docs docs-html
docs-pdf:
	BASEPATH=$(BASEPATH) $(MAKE) -C Docs docs-pdf
docs-ps:
	BASEPATH=$(BASEPATH) $(MAKE) -C Docs docs-ps
docs-man:
	BASEPATH=$(BASEPATH) $(MAKE) -C Docs docs-man
docs-online:
	BASEPATH=$(BASEPATH) $(MAKE) -C Docs docs-online
docs-dist:
	BASEPATH=$(BASEPATH) $(MAKE) -C Docs docs-dist
docs-clean:
	$(MAKE) -C Docs docs-clean

