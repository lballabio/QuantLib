AUTOMAKE_OPTIONS=foreign

SUBDIRS=Sources Include SWIG Python

EXTRA_DIST=News.txt Contributors.txt History.txt LICENSE.TXT \
	Readme.txt Mac/README.txt Python/README.txt \
	Win/README.txt SWIG/README.txt \
	Python/Tests/TestUnit.py \
	Python/Tests/american_option.py \
	Python/Tests/barrier_option.py \
	Python/Tests/date.py \
	Python/Tests/distributions.py \
	Python/Tests/european_option.py \
	Python/Tests/european_with_dividends.py \
	Python/Tests/himalaya_option.py \
	Python/Tests/implied_volatility.py \
	Python/Tests/montecarlo_pricers.py \
	Python/Tests/plain_basket_option.py \
	Python/Tests/random_generators.py \
	Python/Tests/risk_statistics.py \
	Python/Tests/statistics.py \
	Python/Tests/term_structures.py \
	Python/Makefile.am \
	Python/Makefile.in \
	Python/setup.py \
	Mac/QuantLib.prj \
	Mac/QuantLib.prj.exp \
	Mac/README.txt \
	Win/QuantLib.dsw \
	Win/QuantLib.dsp \
	Win/PyQuantLib.dsp \
	Win/PyWrap.bat \
	Win/QuantLibc.def \
	Win/README.txt \
	Win/makefile.mak \
	Docs/README.txt \
	Docs/quantlib.doxy \
	Docs/footer.html \
	Examples/history_iterators.cpp

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
	$(SWIG) -ruby -c++ -ISWIG -o Ruby/quantlib_wrap.cpp SWIG/QuantLib.i
	$(RUBY) -CRuby extconf.rb
	$(MAKE) -C Ruby
else
ruby:
	echo You need to install swig in order to generate wrappers
endif
ruby-install:
	$(MAKE) -C Ruby install
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
docs:
	$(MAKE) -C Docs docs
else
docs:
	echo You need to install doxygen to generate documentation.
endif

