
# $Id$

#AUTOMAKE_OPTIONS=foreign dist-bzip2
AUTOMAKE_OPTIONS=foreign
SUBDIRS=ql Docs Examples
CFLAGS=-Wall

EXTRA_DIST = \
    Authors.txt \
    ChangeLog.txt \
	Contributors.txt \
	History.txt \
	INSTALL.txt \
	LICENSE.TXT \
	makefile.mak \
	News.txt \
	QuantLib.dep \
	QuantLib.dsp \
	QuantLib.dsw \
	QuantLib.mak \
	QuantLib.nsi \
	Readme.txt \
	TODO.txt \
	updateproject.sh

dist-hook:
	mkdir $(distdir)/lib
	mkdir $(distdir)/lib/Win32
	mkdir $(distdir)/lib/Win32/Borland
	mkdir $(distdir)/lib/Win32/VisualStudio
	cp -p $(srcdir)/lib/README.txt $(distdir)/lib/README.txt
	cp -p $(srcdir)/lib/Win32/Borland/README.txt $(distdir)/lib/Win32/Borland/README.txt
	cp -p $(srcdir)/lib/Win32/VisualStudio/README.txt $(distdir)/lib/Win32/VisualStudio/README.txt
	mkdir $(distdir)/Docs/Examples
	cp $(srcdir)/Docs/Examples/*.cpp $(distdir)/Docs/Examples/
	mkdir $(distdir)/Docs/images
	cp $(srcdir)/Docs/images/QL-largish.jpg $(distdir)/Docs/images/
	cp $(srcdir)/Docs/images/QL-small-notitle.jpg $(distdir)/Docs/images/
	cp $(srcdir)/Docs/images/QL-largish.eps $(distdir)/Docs/images/
	cp $(srcdir)/Docs/images/QL-largish.pdf $(distdir)/Docs/images/

if DOXYGEN_FOUND
docs-all:
	$(MAKE) -C Docs docs-all
docs-html:
	$(MAKE) -C Docs docs-html
docs-ps:
	$(MAKE) -C Docs docs-ps
docs-pdf:
	$(MAKE) -C Docs docs-pdf
docs-man:
	$(MAKE) -C Docs docs-man
else
docs-all:
	echo You need to install doxygen in order to generate documentation.
docs-html:
	echo You need to install doxygen in order to generate documentation.
docs-ps:
	echo You need to install doxygen in order to generate documentation.
docs-pdf:
	echo You need to install doxygen in order to generate documentation.
docs-man:
	echo You need to install doxygen in order to generate documentation.
endif
docs-clean:
	$(MAKE) -C Docs docs-clean

