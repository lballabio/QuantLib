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
   Revision 1.30  2001/03/30 15:45:42  lballabio
   Still working on make dist (and added IntVector and DoubleVector to Ruby module)

   Revision 1.29  2001/03/28 13:33:51  lballabio
   Generated distribution almost complete (and added random generators to Ruby module)

   Revision 1.28  2001/03/27 17:39:39  lballabio
   Making sure dist target is complete (and added distributions to Ruby module)

   Revision 1.27  2001/03/26 12:41:23  lballabio
   Added ruby, ruby-install and ruby-test targets to makefile (and added calendars to Ruby module in the meantime)

   Revision 1.26  2001/03/23 15:01:19  lballabio
   Added targets python and python-install which actually build and install the module, and renamed target test to python-test

   Revision 1.25  2001/03/21 15:04:06  lballabio
   Started playing with Ruby

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

%except(ruby) {
    try {
        $function
    } catch (IndexError& e) {
        rb_raise(rb_eIndexError,e.what());
    } catch (Error& e) {
        rb_raise(rb_eStandardError,e.what());
    } catch (std::exception& e) {
        rb_raise(rb_eStandardError,e.what());
    } catch (...) {
        rb_raise(rb_eStandardError,"unknown error");
    }
}

#if defined(SWIGRUBY)
%include Calendars.i
%include Date.i
%include Distributions.i
%include RandomGenerators.i
%include Statistics.i
%include String.i
%include Vectors.i
#else
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


#endif


