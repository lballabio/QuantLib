AUTOMAKE_OPTIONS=foreign

SUBDIRS=Sources Include Docs SWIG Python Ruby Examples

EXTRA_DIST=News.txt Authors.txt Contributors.txt \
	History.txt LICENSE.TXT Readme.txt \
	updateproject.sh makefile.mak \
	README-win.txt QuantLib.dsp \
	README-mac.txt QuantLib.prj QuantLib.prj.exp

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


