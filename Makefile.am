
SUBDIRS=config ql man lib Docs Examples test-suite

bin_SCRIPTS = quantlib-config

# M4 macro file for inclusion with autoconf
m4datadir = $(datadir)/aclocal
m4data_DATA = quantlib.m4

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
	Readme.txt \
	TODO.txt \
	bootstrap \
	makefile.mak \
	quantlib.m4

examples:
	$(MAKE) -C Examples examples

docs-all:
	$(MAKE) -C Docs docs-all
docs-html:
	$(MAKE) -C Docs docs-html
docs-html-online:
	$(MAKE) -C Docs docs-html-online
docs-pdf:
	$(MAKE) -C Docs docs-pdf
docs-man:
	$(MAKE) -C Docs docs-man
docs-clean:
	$(MAKE) -C Docs docs-clean


