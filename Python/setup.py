
from distutils.core import setup, Extension

predir = "/usr/local/include/QuantLib"

setup ( name = "pyQuantLib", \
	version = "0.1.1", \
	maintainer = "Enrico Sirola", \
	maintainer_email = "enri@users.sourceforge.net", \
	url = "http://quantlib.sourceforge.net", \
	py_modules = ["QuantLib"],
	ext_modules = [ \
		Extension ( "QuantLibc", \
			["quantlib_wrap.cpp"], \
			libraries = ["QuantLib"], \
			include_dirs = [predir, \
					"%s/Calendars" % predir, \
					"%s/Currencies" % predir, \
					"%s/DayCounters" % predir, \
					"%s/FiniteDifferences" % predir, \
					"%s/Instruments" % predir, \
					"%s/Math" % predir, \
					"%s/Patterns" % predir, \
					"%s/Pricers" % predir, \
					"%s/Solvers1D" % predir, \
					"%s/TermStructures" % predir, \
					"%s/MonteCarlo" % predir, \
					"%s/Utilities" % predir \
					] \
				) \
			] \
		)
