
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

#ifndef quantlib_day_counters_i
#define quantlib_day_counters_i

%module DayCounters

%{
#include "quantlib.h"
%}

#if !defined(SWIGPYTHON)
#if !defined(PYTHON_WARNING_ISSUED)
#define PYTHON_WARNING_ISSUED
%echo "Warning: this is a Python module!!"
%echo "Exporting it to any other language is not advised as it could lead to unpredicted results."
#endif
#endif

%include Date.i

%{
using QuantLib::DayCounter;
using QuantLib::Handle;
typedef Handle<DayCounter> DayCounterHandle;
%}

// export Handle<DayCounter>
%name(DayCounter) class DayCounterHandle {
  public:
	// no constructor - forbid explicit construction
	~DayCounterHandle();
};

// replicate the DayCounter interface
%addmethods DayCounterHandle {
	int dayCount(const Date& d1, const Date& d2) {
		return (*self)->dayCount(d1,d2);
	}
	Time yearFraction(const Date& d1, const Date& d2) {
		return (*self)->yearFraction(d1,d2);
	}
	#if defined (SWIGPYTHON)
	char* __str__() {
		static char temp[256];
		sprintf(temp,"%s day counter",(*self)->name().c_str());
		return temp;
	}
	char* __repr__() {
		static char temp[256];
		sprintf(temp,"<%s day counter>",(*self)->name().c_str());
		return temp;
	}
	int __cmp__(const DayCounterHandle& other) {
		return ((*self) == other ? 0 : 1);
	}
	int __nonzero__() {
		return (IsNull(*self) ? 0 : 1);
	}
	#endif
}

// actual day counters

%{
using QuantLib::DayCounters::Actual360;
using QuantLib::DayCounters::Actual365;
using QuantLib::DayCounters::Thirty360;
using QuantLib::DayCounters::Thirty360European;
using QuantLib::DayCounters::Thirty360Italian;

DayCounterHandle NewActual360()			{ return DayCounterHandle(new Actual360); }
DayCounterHandle NewActual365()			{ return DayCounterHandle(new Actual365); }
DayCounterHandle NewThirty360()			{ return DayCounterHandle(new Thirty360); }
DayCounterHandle NewThirty360European()	{ return DayCounterHandle(new Thirty360European); }
DayCounterHandle NewThirty360Italian()	{ return DayCounterHandle(new Thirty360Italian); }
%}

%name(Actual360)			DayCounterHandle NewActual360();
%name(Actual365)			DayCounterHandle NewActual365();
%name(Thirty360)			DayCounterHandle NewThirty360();
%name(Thirty360European)	DayCounterHandle NewThirty360European();
%name(Thirty360Italian)		DayCounterHandle NewThirty360Italian();


#endif
