
# $Id$
# $Source$
# $Log$
# Revision 1.38  2001/08/23 12:59:45  nando
# makedist now distributes examples
#

AUTOMAKE_OPTIONS=foreign

SUBDIRS=Sources Include Docs

EXTRA_DIST=Authors.txt \
	Contributors.txt \
	History.txt \
	INSTALL.txt \
	LICENSE.TXT \
	makefile.mak \
	News.txt \
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
	cp $(srcdir)/Docs/Examples/*.dsw $(distdir)/Docs/Examples/
	cp $(srcdir)/Docs/Examples/*.mak $(distdir)/Docs/Examples/
	cp $(srcdir)/Docs/Examples/*.txt $(distdir)/Docs/Examples/
	mkdir $(distdir)/Docs/Examples/HedgingError
	cp $(srcdir)/Docs/Examples/HedgingError/*.cpp $(distdir)/Docs/Examples/HedgingError/
	cp $(srcdir)/Docs/Examples/HedgingError/*.dsw $(distdir)/Docs/Examples/HedgingError/
	cp $(srcdir)/Docs/Examples/HedgingError/*.mak $(distdir)/Docs/Examples/HedgingError/
	cp $(srcdir)/Docs/Examples/HedgingError/*.txt $(distdir)/Docs/Examples/HedgingError/
	mkdir $(distdir)/Docs/Examples/Parities
	cp $(srcdir)/Docs/Examples/Parities/*.cpp $(distdir)/Docs/Examples/Parities/
	cp $(srcdir)/Docs/Examples/Parities/*.dsw $(distdir)/Docs/Examples/Parities/
	cp $(srcdir)/Docs/Examples/Parities/*.mak $(distdir)/Docs/Examples/Parities/
	cp $(srcdir)/Docs/Examples/Parities/*.txt $(distdir)/Docs/Examples/Parities/
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

