
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
 * QuantLib license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
*/

#ifndef quantlib_term_structures_i
#define quantlib_term_structures_i

%module TermStructures

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

%include Date.i
%include Calendars.i
%include DayCounters.i
%include Financial.i
%include Currencies.i

%{
using QuantLib::TermStructure;
using QuantLib::Handle;
typedef Handle<TermStructure> TermStructureHandle;
%}

// export Handle<TermStructure>
%name(TermStructure) class TermStructureHandle {
  public:
	// no constructor - forbid explicit construction
	~TermStructureHandle();
};

// replicate the TermStructure interface
%addmethods TermStructureHandle {
	TermStructureHandle clone() {
		return (*self)->clone();
	}
	CurrencyHandle currency() {
		return (*self)->currency();
	}
	Date todaysDate() {
		return (*self)->todaysDate();
	}
	Date settlementDate() {
		return (*self)->settlementDate();
	}
	CalendarHandle calendar() {
		return (*self)->calendar();
	}
	Date maxDate() {
		return (*self)->maxDate();
	}
	Date minDate() {
		return (*self)->minDate();
	}
	Rate zeroYield(const Date& d) {
		return (*self)->zeroYield(d);
	}
	DiscountFactor discount(const Date& d) {
		return (*self)->discount(d);
	}
	Rate forward(const Date& d) {
		return (*self)->forward(d);
	}
	#if defined (SWIGPYTHON)
	int __nonzero__() {
		return (IsNull(*self) ? 0 : 1);
	}
	#endif
}

// implied term structure

%{
using QuantLib::ImpliedTermStructure;

TermStructureHandle NewImpliedTermStructure(TermStructureHandle curve, 
  Date evaluationDate) {
	return Handle<TermStructure>(
	  new ImpliedTermStructure(curve,evaluationDate));
}
%}

%name(ImpliedTermStructure) TermStructureHandle 
NewImpliedTermStructure(TermStructureHandle curve, Date evaluationDate);


// spreaded term structure

%{
using QuantLib::SpreadedTermStructure;

TermStructureHandle NewSpreadedTermStructure(TermStructureHandle curve, 
  Spread spread) {
	return Handle<TermStructure>(new SpreadedTermStructure(curve,spread));
}
%}

%name(SpreadedTermStructure) TermStructureHandle NewSpreadedTermStructure(TermStructureHandle curve, Spread spread);

// flat forward curve

%{
using QuantLib::TermStructures::FlatForward;

TermStructureHandle NewFlatForward(CurrencyHandle currency, 
  DayCounterHandle dayCounter, Date today, Rate forward) {
	return Handle<TermStructure>(
	  new FlatForward(currency,dayCounter,today,forward));
}
%}

%name(FlatForward) TermStructureHandle NewFlatForward(CurrencyHandle currency,
  DayCounterHandle dayCounter, Date today, Rate forward);



// piecewise constant forward curve on deposits

%{
using QuantLib::TermStructures::PiecewiseConstantForwards;
using QuantLib::Deposit;
%}

// deposit

class Deposit {
  public:
	Deposit(Date maturity, Rate rate, DayCounterHandle dayCounter);
	~Deposit();
	Date maturity() const;
	Rate rate() const;
	DayCounterHandle dayCounter();
};

// typemap Python list of deposits to std::vector<Deposit>

%{
typedef std::vector<Deposit> DepositList;
%}

%typemap(python,in) DepositList, DepositList *, const DepositList & {
	if (PyTuple_Check($source)) {
		int size = PyTuple_Size($source);
		$target = new std::vector<Deposit>(size);
		for (int i=0; i<size; i++) {
			Deposit* d;
			PyObject* o = PyTuple_GetItem($source,i);
			if ((SWIG_ConvertPtr(o,(void **) &d,
			  (swig_type_info *)SWIG_TypeQuery("Deposit *"),1)) != -1) {
				(*$target)[i] = *d;
			} else {
				PyErr_SetString(PyExc_TypeError,"tuple must contain deposits");
				delete $target;
				return NULL;
			}
		}
	} else if (PyList_Check($source)) {
		int size = PyList_Size($source);
		$target = new std::vector<Deposit>(size);
		for (int i=0; i<size; i++) {
			Deposit* d;
			PyObject* o = PyList_GetItem($source,i);
			if ((SWIG_ConvertPtr(o,(void **) &d,
			  (swig_type_info *)SWIG_TypeQuery("Deposit *"),1)) != -1) {
				(*$target)[i] = *d;
			} else {
				PyErr_SetString(PyExc_TypeError,"list must contain deposits");
				delete $target;
				return NULL;
			}
		}
	} else {
		PyErr_SetString(PyExc_TypeError,"not a sequence");
		return NULL;
	}
};

%typemap(python,freearg) DepositList, DepositList *, const DepositList & {
	delete $source;
};

// piecewise constant forward generation

%{
TermStructureHandle NewPiecewiseConstantForwards(CurrencyHandle currency, 
  DayCounterHandle dayCounter, Date today, DepositList deposits) {
	return Handle<TermStructure>(
	  new PiecewiseConstantForwards(currency,dayCounter,today,deposits));
}
%}

%name(PiecewiseConstantForwards) TermStructureHandle NewPiecewiseConstantForwards(CurrencyHandle currency, 
  DayCounterHandle dayCounter, Date today, DepositList deposits);


#endif

