
# $Id$

SUBDIRS=config ql man lib Docs Examples

bin_SCRIPTS = quantlib-config

# M4 macro file for inclusion with autoconf
m4datadir = $(prefix)/aclocal
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


docs-all:
	$(MAKE) -C Docs docs-all
docs-html:
	$(MAKE) -C Docs docs-html
docs-html-online:
	$(MAKE) -C docs-html-online
docs-pdf:
	$(MAKE) -C docs-pdf
docs-man:
	$(MAKE) -C docs-man
docs-clean:
	$(MAKE) -C docs-clean


