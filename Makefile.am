
# $Id$
# $Source$
# $Log$
# Revision 1.30  2001/06/05 10:56:08  lballabio
# Added docs-clean target
#
# Revision 1.29  2001/05/28 14:57:54  lballabio
# Removed files removed from distribution list also
#
# Revision 1.28  2001/05/24 12:52:01  nando
# smoothing #include xx.hpp
#

AUTOMAKE_OPTIONS=foreign

SUBDIRS=Sources Include Docs Examples

EXTRA_DIST=Authors.txt \
	Contributors.txt \
	History.txt \
	LICENSE.TXT \
	makefile.mak \
	News.txt \
	QuantLib.dsp \
	QuantLib.dsw \
	QuantLib.mak \
	QuantLib.nsi \
	Readme.txt \
	README-win.txt \
	README-mac.txt \
	updateproject.sh

dist-hook:
	mkdir $(distdir)/lib
	mkdir $(distdir)/lib/Win32
	mkdir $(distdir)/lib/Win32/Borland
	mkdir $(distdir)/lib/Win32/VisualStudio
	cp -p $(srcdir)/lib/README.txt $(distdir)/lib/README.txt
	cp -p $(srcdir)/lib/Win32/Borland/README.txt $(distdir)/lib/Win32/Borland/README.txt
	cp -p $(srcdir)/lib/Win32/VisualStudio/README.txt $(distdir)/lib/Win32/VisualStudio/README.txt


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

