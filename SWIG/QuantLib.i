
/* 
Copyright (C) 2000 Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

%module QuantLib

#if !defined(SWIGPYTHON)
#if !defined(PYTHON_WARNING_ISSUED)
#define PYTHON_WARNING_ISSUED
%echo "Warning: this is a Python module!!"
%echo "Exporting it to any other language is not advised as it could lead to unpredicted results."
#endif
#endif

%{
#include "qldefines.h"
%}

%except(python) {
	try {
		$function
	} catch (std::exception& e) {
		PyErr_SetString(PyExc_Exception,e.what());
		return NULL;
	} catch (...) {
		PyErr_SetString(PyExc_Exception,"unknown error");
		return NULL;
	}
}

%include Date.i
%include Calendars.i
%include DayCounters.i
%include Currencies.i
%include Financial.i
%include Options.i
%include Instruments.i
%include Operators.i
%include Pricers.i
%include Solvers1D.i
%include TermStructures.i
%include Statistics.i

