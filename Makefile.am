
# $Id$

#AUTOMAKE_OPTIONS=foreign dist-bzip2

AUTOMAKE_OPTIONS=foreign
SUBDIRS=ql Examples
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
	News.txt \
	QuantLib.dsp \
	QuantLib.dsw \
	QuantLib.mak \
	QuantLib.nsi \
	Readme.txt \
	TODO.txt \
	bootstrap \
	makefile.mak

dist-hook:
    mkdir $(distdir)/config
    cp $(srcdir)/config/* $(distdir)/config/*
	mkdir $(distdir)/lib
	mkdir $(distdir)/lib/Win32
	mkdir $(distdir)/lib/Win32/Borland
	mkdir $(distdir)/lib/Win32/VisualStudio
	cp -p $(srcdir)/lib/README.txt $(distdir)/lib/README.txt
	cp -p $(srcdir)/lib/Win32/Borland/README.txt $(distdir)/lib/Win32/Borland/README.txt
	cp -p $(srcdir)/lib/Win32/VisualStudio/README.txt $(distdir)/lib/Win32/VisualStudio/README.txt
	mkdir $(distdir)/Docs
	cp $(srcdir)/Docs/*.in $(distdir)/Docs
	cp $(srcdir)/Docs/*.am  $(distdir)/Docs
	cp $(srcdir)/Docs/*.mak $(distdir)/Docs
	cp $(srcdir)/Docs/*.tex $(distdir)/Docs
	cp $(srcdir)/Docs/*.css $(distdir)/Docs
	cp $(srcdir)/Docs/*.docs $(distdir)/Docs
	cp $(srcdir)/Docs/*.doxy $(distdir)/Docs
	cp $(srcdir)/Docs/*.html $(distdir)/Docs
	cp $(srcdir)/Docs/README.txt $(distdir)/Docs
	mkdir $(distdir)/Docs/Examples
	cp $(srcdir)/Docs/Examples/*.cpp $(distdir)/Docs/Examples/
	mkdir $(distdir)/Docs/images
	cp $(srcdir)/Docs/images/QL-largish.bmp $(distdir)/Docs/images/
	cp $(srcdir)/Docs/images/QL-largish.jpg $(distdir)/Docs/images/
	cp $(srcdir)/Docs/images/QL-small-notitle.jpg $(distdir)/Docs/images/
	cp $(srcdir)/Docs/images/QL-largish.eps $(distdir)/Docs/images/
	cp $(srcdir)/Docs/images/QL-largish.pdf $(distdir)/Docs/images/
	cp $(srcdir)/Docs/images/sflogo.png $(distdir)/Docs/images/

