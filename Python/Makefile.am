
SUBDIRS=Tests COM

EXTRA_DIST=QuantLib.py quantlib_wrap.cpp QuantLibc.def \
	README.txt setup.py makefile.mak

SWIG_INCLUDEDIR = ../SWIG

#lib_LTLIBRARIES = libQuantLibc.la

#LIBS = @LIBS@ @PYLIB@ -lQuantLib -L../Sources

#libQuantLibc_la_SOURCES = quantlib_wrap.cpp

python: quantlib_wrap.cpp
	$(PYTHON) setup.py build

python-install:
	$(PYTHON) setup.py install

quantlib_wrap.cpp:  $(SWIG_INCLUDEDIR)/QuantLib.i \
	$(SWIG_INCLUDEDIR)/Date.i \
	$(SWIG_INCLUDEDIR)/Calendars.i \
	$(SWIG_INCLUDEDIR)/DayCounters.i \
	$(SWIG_INCLUDEDIR)/Currencies.i \
	$(SWIG_INCLUDEDIR)/Financial.i \
	$(SWIG_INCLUDEDIR)/Options.i \
	$(SWIG_INCLUDEDIR)/Instruments.i \
	$(SWIG_INCLUDEDIR)/Operators.i \
	$(SWIG_INCLUDEDIR)/Pricers.i \
	$(SWIG_INCLUDEDIR)/Solvers1D.i \
	$(SWIG_INCLUDEDIR)/TermStructures.i \
	$(SWIG_INCLUDEDIR)/Vectors.i \
	$(SWIG_INCLUDEDIR)/BoundaryConditions.i \
	$(SWIG_INCLUDEDIR)/Statistics.i \
	$(SWIG_INCLUDEDIR)/History.i \
	$(SWIG_INCLUDEDIR)/Distributions.i \
	$(SWIG_INCLUDEDIR)/MontecarloPricers.i
	$(SWIG) -python -c++ -shadow -keyword -opt -I$(SWIG_INCLUDEDIR) -o quantlib_wrap.cpp $(SWIG_INCLUDEDIR)/QuantLib.i

