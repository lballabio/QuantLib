
/* 
Copyright (C) 2000 F.Ametrano, L.Ballabio, A.Benin, M.Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

#ifndef quantlib_day_counters_i
#define quantlib_day_counters_i

%module DayCounters

#if !defined(SWIGPYTHON)
#if !defined(PYTHON_WARNING_ISSUED)
#define PYTHON_WARNING_ISSUED
%echo "Warning: this is a Python module!!"
%echo "Exporting it to any other language is not advised as it could lead to unpredicted results."
#endif
#endif

%include Date.i

%{
#include "daycounter.h"
#include "actual360.h"
#include "actual365.h"
#include "actualactual.h"
#include "thirty360.h"
#include "thirty360european.h"
#include "thirty360italian.h"

QL_USING(QuantLib,Handle)
QL_USING(QuantLib,DayCounter)
typedef Handle<DayCounter> DayCounterHandle;
%}

// export Handle<DayCounter>
%name(DayCounter) class DayCounterHandle {
  public:
	// abstract class - forbid explicit construction
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
QL_USING(QuantLib::DayCounters,Actual360)
QL_USING(QuantLib::DayCounters,Actual365)
QL_USING(QuantLib::DayCounters,Thirty360)
QL_USING(QuantLib::DayCounters,Thirty360European)
QL_USING(QuantLib::DayCounters,Thirty360Italian)

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
