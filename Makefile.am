AUTOMAKE_OPTIONS=foreign

SUBDIRS=Sources Include Docs SWIG Python Ruby Examples

EXTRA_DIST=Authors.txt \
    Contributors.txt \
	History.txt \
	LICENSE.TXT \
	makefile.mak \
    News.txt \
	QuantLib.dsp \
	QuantLib.dsw \
	QuantLib.mak \
	QuantLib.prj \
	QuantLib.prj.exp \
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

if PYTHON_FOUND
if SWIG_FOUND
python:
	$(MAKE) -C Python python
else
python:
	echo You need to install swig in order to generate wrappers
endif
python-install:
	$(MAKE) -C Python python-install
python-test:
	$(MAKE) -C Python/Tests python-test
else
python:
	echo You need to install python in order to build this module
python-install:
	echo You need to install python in order to install this module
python-test:
	echo You need to install python in order to test this module
endif


if RUBY_FOUND
if SWIG_FOUND
ruby:
	$(MAKE) -C Ruby ruby
else
ruby:
	echo You need to install swig in order to generate wrappers
endif
ruby-install:
	$(MAKE) -C Ruby ruby-install
ruby-test:
	$(MAKE) -C Ruby/Tests ruby-test
else
ruby:
	echo You need to install ruby in order to build this module
ruby-install:
	echo You need to install ruby in order to install this module
ruby-test:
	echo You need to install ruby in order to test this module
endif


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


