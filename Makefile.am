
# $Id$
# $Source$
# $Log$
# Revision 1.36  2001/07/25 07:42:29  nando
# updated
#
# Revision 1.35  2001/07/25 07:36:08  nando
# updated
#
# Revision 1.34  2001/07/24 16:59:33  nando
# documentation revised

AUTOMAKE_OPTIONS=foreign

SUBDIRS=Sources Include Docs Examples

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
	cp $(srcdir)/Docs/Examples/*.cpp $(distdir)/Docs/Examples/
	mkdir $(distdir)/Docs/images
	cp $(srcdir)/Docs/images/QL-largish.jpg $(distdir)/Docs/images/
	cp $(srcdir)/Docs/images/QL-small-notitle.jpg $(distdir)/Docs/images/
	cp $(srcdir)/Docs/images/QL-largish.eps $(distdir)/Docs/images/
	cp $(srcdir)/Docs/images/QL-largish.pdf $(distdir)/Docs/images/

if DOXYGEN_FOUND
docs-html:
	$(MAKE) -C Docs docs-html
docs-ps:
	$(MAKE) -C Docs docs-ps
docs-pdf:
	$(MAKE) -C Docs docs-pdf
docs-man:
	$(MAKE) -C Docs docs-man
else
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

