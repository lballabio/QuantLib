
/* 
Copyright (C) 2000 F.Ametrano, L.Ballabio, A.Benin, M.Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

#ifndef quantlib_term_structures_i
#define quantlib_term_structures_i

%module TermStructures

#if !defined(SWIGPYTHON)
#if !defined(PYTHON_WARNING_ISSUED)
#define PYTHON_WARNING_ISSUED
%echo "Warning: this is a Python module!!"
%echo "Exporting it to any other language is not advised as it could lead to unpredicted results."
#endif
#endif

%include Date.i
%include Calendars.i
%include DayCounters.i
%include Financial.i
%include Currencies.i

%{
#include "termstructure.h"
#include "handle.h"

QL_USING(QuantLib,TermStructure)
QL_USING(QuantLib,Handle)
typedef Handle<TermStructure> TermStructureHandle;
%}

// export Handle<TermStructure>
%name(TermStructure) class TermStructureHandle {
  public:
	// abstract class - forbid explicit construction
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
	Yield zeroYield(const Date& d) {
		return (*self)->zeroYield(d);
	}
	DiscountFactor discount(const Date& d) {
		return (*self)->discount(d);
	}
	Yield forward(const Date& d) {
		return (*self)->forward(d);
	}
	#if defined (SWIGPYTHON)
	char* __str__() {
		static char temp[256];
		sprintf(temp,"Term structure");
		return temp;
	}
	char* __repr__() {
		static char temp[256];
		sprintf(temp,"<Term structure>");
		return temp;
	}
	int __nonzero__() {
		return (IsNull(*self) ? 0 : 1);
	}
	#endif
}

// piecewise constant forward curve on deposits

%{
#include "piecewiseconstantforwards.h"

QL_USING(QuantLib::TermStructures,PiecewiseConstantForwards)
QL_USING(QuantLib,Deposit)
%}

// deposit

class Deposit {
  public:
	Deposit(Date maturity, Yield rate, DayCounterHandle dayCounter);
	~Deposit();
	Date maturity() const;
	Yield rate() const;
	DayCounterHandle dayCounter();
};

// typemap Python list of deposits to std::vector<Deposit>

%{
	typedef std::vector<Deposit> DepositList;
%}

%typemap(python,in) DepositList, DepositList * {
	if (PyTuple_Check($source)) {
		int size = PyTuple_Size($source);
		$target = new std::vector<Deposit>(size);
		for (int i=0; i<size; i++) {
			Deposit* d;
			PyObject* o = PyTuple_GetItem($source,i);
			if ((SWIG_ConvertPtr(o,(void **) &d,(swig_type_info *)SWIG_TypeQuery("Deposit *"),1)) != -1) {
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
			if ((SWIG_ConvertPtr(o,(void **) &d,(swig_type_info *)SWIG_TypeQuery("Deposit *"),1)) != -1) {
				(*$target)[i] = *d;
			} else {
				PyErr_SetString(PyExc_TypeError,"tuple must contain deposits");
				delete $target;
				return NULL;
			}
		}
	} else {
		PyErr_SetString(PyExc_TypeError,"not a sequence");
		return NULL;
	}
};

%typemap(python,freearg) DepositList, DepositList * {
	delete $source;
};

// piecewise constant forward generation

%{
TermStructureHandle NewPiecewiseConstantForwards(CurrencyHandle currency, DayCounterHandle dayCounter, 
  Date today, DepositList deposits) {
	return Handle<TermStructure>(new PiecewiseConstantForwards(currency,dayCounter,today,deposits));
}
%}

%name(PiecewiseConstantForwards)	TermStructureHandle NewPiecewiseConstantForwards(CurrencyHandle currency, 
  DayCounterHandle dayCounter, Date today, DepositList deposits);


#endif

