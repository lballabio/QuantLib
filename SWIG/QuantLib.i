/*
 * Copyright (C) 2000, 2001
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

/* $Source$
   $Log$
   Revision 1.24  2001/03/19 17:30:28  nando
   refactored *.i files inclusion.
   The files are sorted by SWIG debug problem

   Revision 1.23  2001/03/09 12:40:41  lballabio
   Spring cleaning for SWIG interfaces

*/

%module QuantLib

%{
#include "quantlib.h"
%}

#if !defined(SWIGPYTHON)
#if !defined(PYTHON_WARNING_ISSUED)
#define PYTHON_WARNING_ISSUED
%echo "Warning: QuantLib is a Python module!!"
%echo "Exporting it to any other language is not advised"
%echo "as it could lead to unpredicted results."
#endif
#endif

%{
using QuantLib::Error;
using QuantLib::IndexError;
%}

%except(python) {
    try {
        $function
    } catch (IndexError& e) {
        PyErr_SetString(PyExc_IndexError,e.what());
        return NULL;
    } catch (Error& e) {
        PyErr_SetString(PyExc_Exception,e.what());
        return NULL;
    } catch (std::exception& e) {
        PyErr_SetString(PyExc_Exception,e.what());
        return NULL;
    } catch (...) {
        PyErr_SetString(PyExc_Exception,"unknown error");
        return NULL;
    }
}

// PLEASE ADD ANY NEW *.i FILE AT THE BOTTOM, NOT HERE
// the following files have no problem with SWIG in debug mode
%include Barrier.i
%include Financial.i
%include Options.i
%include RandomGenerators.i
%include String.i

// PLEASE ADD ANY NEW *.i FILE AT THE BOTTOM, NOT HERE
// the following files break SWIG in debug mode
%include BoundaryConditions.i
%include Distributions.i
%include Date.i
%include QLArray.i
%include Vectors.i

// PLEASE ADD ANY NEW *.i FILE AT THE BOTTOM, NOT HERE
// the following files depends on files that break SWIG in debug mode
%include Calendars.i
%include Currencies.i
%include DayCounters.i
%include History.i
%include Instruments.i
%include Interpolation.i
%include Matrix.i
%include MontecarloPricers.i
%include MontecarloTools.i
%include Operators.i
%include Pricers.i
%include RiskStatistics.i
%include Solvers1D.i
%include Statistics.i
%include TermStructures.i

// PLEASE ADD ANY NEW *.i FILE HERE
