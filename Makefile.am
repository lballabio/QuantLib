
# $Id$

#AUTOMAKE_OPTIONS=foreign dist-bzip2

AUTOMAKE_OPTIONS=foreign
SUBDIRS=config ql man lib Docs Examples patches
CFLAGS=-Wall

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
	MACOSX.README \
	News.txt \
	QuantLib.dsp \
	QuantLib.dsw \
	QuantLib.mak \
	QuantLib.nsi \
	Readme.txt \
	TODO.txt \
	bootstrap \
	darwin.setup \
	makefile.mak \
	quantlib.m4


tags:
	cd ql && find . -name '*.[hc]pp' -print | etags --members -


docs-all:
	$(MAKE) -C Docs docs-all
docs-html:
	$(MAKE) -C Docs docs-html
docs-html-online:
	$(MAKE) -C docs-html-online
docs-ps:
	$(MAKE) -C docs-ps
docs-pdf:
	$(MAKE) -C docs-pdf
docs-man:
	$(MAKE) -C docs-man
docs-clean:
	$(MAKE) -C docs-clean


