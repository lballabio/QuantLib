/*
 * Copyright (C) 2000
 * Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.sourceforge.net/
 *
 * QuantLib is free software and you are allowed to use, copy, modify, merge,
 * publish, distribute, and/or sell copies of it under the conditions stated
 * in the QuantLib License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
 *
 * You should have received a copy of the license along with this file;
 * if not, contact ferdinando@ametrano.net
 *
 * QuantLib license is also available at
 * http://quantlib.sourceforge.net/LICENSE.TXT
*/

%module QuantLib

%{
#include "quantlib.h"
%}

#if !defined(SWIGPYTHON)
#if !defined(PYTHON_WARNING_ISSUED)
#define PYTHON_WARNING_ISSUED
%echo "Warning: this is a Python module!!"
%echo "Exporting it to any other language is not advised"
%echo "as it could lead to unpredicted results."
#endif
#endif

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

%include BoundaryConditions.i
%include Calendars.i
%include Currencies.i
%include Date.i
%include DayCounters.i
%include Distributions.i
%include Financial.i
%include History.i
%include Instruments.i
%include Interpolation.i
%include Matrix.i
%include MontecarloPricers.i
%include Operators.i
%include Options.i
%include Pricers.i
%include RandomGenerators.i
%include RiskStatistics.i
%include Solvers1D.i
%include Statistics.i
%include String.i
%include TermStructures.i
%include Vectors.i
