
/* 
Copyright (C) 2000 F.Ametrano, L.Ballabio, A.Benin, M.Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

#ifndef quantlib_calendar_i
#define quantlib_calendar_i

%module Calendar

#if !defined(SWIGPYTHON)
#if !defined(PYTHON_WARNING_ISSUED)
#define PYTHON_WARNING_ISSUED
%echo "Warning: this is a Python module!!"
%echo "Exporting it to any other language is not advised as it could lead to unpredicted results."
#endif
#endif

%include Date.i

%{
#include "calendar.h"

QL_USING(QuantLib,Handle)
QL_USING(QuantLib,Calendar)
typedef Handle<Calendar> CalendarHandle;

QL_USING(QuantLib,Following)
QL_USING(QuantLib,ModifiedFollowing)
QL_USING(QuantLib,IsNull)
%}

// export Handle<Calendar>
%name(Calendar) class CalendarHandle {
  public:
	// abstract class - forbid explicit construction
	~CalendarHandle();
};

// replicate the Calendar interface
%addmethods CalendarHandle {
	bool isBusinessDay(const Date& d) {
		return (*self)->isBusinessDay(d);
	}
	bool isHoliday(const Date& d) {
		return (*self)->isHoliday(d);
	}
	Date roll(const Date& d, bool modified = false) {
		return (*self)->roll(modified ? ModifiedFollowing : Following, d);
	}
	Date advance(const Date& d, int businessDays) {
		return (*self)->advance(d,businessDays);
	}
	#if defined (SWIGPYTHON)
	char* __str__() {
		static char temp[256];
		sprintf(temp,"%s calendar",(*self)->name().c_str());
		return temp;
	}
	char* __repr__() {
		static char temp[256];
		sprintf(temp,"<%s calendar>",(*self)->name().c_str());
		return temp;
	}
	int __cmp__(const CalendarHandle& other) {
		return ((*self) == other ? 0 : 1);
	}
	int __nonzero__() {
		return (IsNull(*self) ? 0 : 1);
	}
	#endif
}

// actual calendars

%{
#include "target.h"
#include "newyork.h"
#include "london.h"
#include "frankfurt.h"
#include "milan.h"

QL_USING(QuantLib::Calendars,TARGET)
QL_USING(QuantLib::Calendars,NewYork)
QL_USING(QuantLib::Calendars,London)
QL_USING(QuantLib::Calendars,Milan)
QL_USING(QuantLib::Calendars,Frankfurt)

CalendarHandle NewTARGET()    { return CalendarHandle(new TARGET); }
CalendarHandle NewNewYork()   { return CalendarHandle(new NewYork); }
CalendarHandle NewLondon()    { return CalendarHandle(new London); }
CalendarHandle NewFrankfurt() { return CalendarHandle(new Frankfurt); }
CalendarHandle NewMilan()     { return CalendarHandle(new Milan); }
%}

%name(TARGET)    CalendarHandle NewTARGET();
%name(NewYork)   CalendarHandle NewNewYork();
%name(London)    CalendarHandle NewLondon();
%name(Frankfurt) CalendarHandle NewFrankfurt();
%name(Milan)     CalendarHandle NewMilan();


#endif
