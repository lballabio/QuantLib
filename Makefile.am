AUTOMAKE_OPTIONS=foreign

SUBDIRS=Sources Include SWIG Python

EXTRA_DIST=News.txt Contributors.txt History.txt LICENSE.TXT \
	Readme.txt Mac/README.txt Python/README.txt \
	Win/README.txt SWIG/README.txt \
	Python/Tests/american_option.py \
	Python/Tests/european_option.py \
	Python/Tests/european_with_dividends.py \
	Python/Tests/impliedVol.py \
	Python/Tests/mcpricers.py \
	Python/Tests/statistics_test.py \
	Python/Tests/testRNGs.py \
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
	 

python:
	$(MAKE) -C Python python
