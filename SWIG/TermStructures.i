
/*
 * Copyright (C) 2000-2001 QuantLib Group
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
 * The license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.sourceforge.net/Authors.txt
*/

/* $Source$
   $Log$
   Revision 1.21  2001/04/06 18:46:21  nando
   changed Authors, Contributors, Licence and copyright header

   Revision 1.20  2001/03/12 17:35:11  lballabio
   Removed global IsNull function - could have caused very vicious loops

   Revision 1.19  2001/03/09 12:40:41  lballabio
   Spring cleaning for SWIG interfaces

*/

#ifndef quantlib_term_structures_i
#define quantlib_term_structures_i

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
	Rate zeroYield(const Date& d, bool extrapolate = false) {
		return (*self)->zeroYield(d, extrapolate);
	}
	DiscountFactor discount(const Date& d, bool extrapolate = false) {
		return (*self)->discount(d, extrapolate);
	}
	Rate forward(const Date& d, bool extrapolate = false) {
		return (*self)->forward(d, extrapolate);
	}
	#if defined (SWIGPYTHON)
	int __nonzero__() {
		return (self->isNull() ? 0 : 1);
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
using QuantLib::DepositRate;
%}

// deposit

class DepositRate {
  public:
	DepositRate(Date maturity, Rate rate, DayCounterHandle dayCounter);
	~DepositRate();
	Date maturity() const;
	Rate rate() const;
	DayCounterHandle dayCounter();
};

// typemap Python list of deposits to std::vector<DepositRate>

%{
typedef std::vector<DepositRate> DepositList;
%}

%typemap(python,in) DepositList, DepositList *, const DepositList & {
	if (PyTuple_Check($source)) {
		int size = PyTuple_Size($source);
		$target = new DepositList(size);
		for (int i=0; i<size; i++) {
			DepositRate* d;
			PyObject* o = PyTuple_GetItem($source,i);
			if ((SWIG_ConvertPtr(o,(void **) &d,
			  (swig_type_info *)SWIG_TypeQuery("DepositRate *"),1)) != -1) {
				(*$target)[i] = *d;
			} else {
				PyErr_SetString(PyExc_TypeError,"tuple must contain deposits");
				delete $target;
				return NULL;
			}
		}
	} else if (PyList_Check($source)) {
		int size = PyList_Size($source);
		$target = new DepositList(size);
		for (int i=0; i<size; i++) {
			DepositRate* d;
			PyObject* o = PyList_GetItem($source,i);
			if ((SWIG_ConvertPtr(o,(void **) &d,
			  (swig_type_info *)SWIG_TypeQuery("DepositRate *"),1)) != -1) {
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

