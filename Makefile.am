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
	Win/PyQuantLib.dsp \
	Win/PyQuantLib.dsw \
	Win/PyWrap.bat \
	Win/QuantLib.dsp \
	Win/QuantLib.dsw \
	Win/QuantLibc.def \
	Win/README.txt \
	Win/makefile.mak \
	Docs/README.txt \
	Docs/offline.doxy \
	Docs/online.doxy \
	Docs/onlinefooter.html \
	Examples/history_iterators.cpp
 

if SWIG_FOUND
python:
	$(MAKE) -C Python python
else
python:
	echo You have to install swig in order to generate wrappers, sorry.
endif

test:
	$(MAKE) -C Python/Tests test

