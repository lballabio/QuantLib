
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

#ifndef quantlib_calendar_i
#define quantlib_calendar_i

%module Calendars

%{
#include "quantlib.h"
%}

#if !defined(SWIGPYTHON)
#if !defined(PYTHON_WARNING_ISSUED)
#define PYTHON_WARNING_ISSUED
%echo "Warning: Calendars is a Python module!!"
%echo "Exporting it to any other language is not advised"
%echo "as it could lead to unpredicted results."
#endif
#endif

%include Date.i
%include String.i

%{
using QuantLib::Handle;
using QuantLib::Calendar;
typedef Handle<Calendar> CalendarHandle;

using QuantLib::IsNull;
%}

// export Handle<Calendar>
%name(Calendar) class CalendarHandle {
  public:
    // no constructor - forbid explicit construction
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
        return (*self)->roll(d,modified);
    }
    Date advance(const Date& d, int n, TimeUnit unit, bool modified = false) {
        return (*self)->advance(d,n,unit,modified);
    }
    #if defined (SWIGPYTHON)
    String __str__() {
        return (*self)->name()+" calendar";
    }
    String __repr__() {
        return "<"+(*self)->name()+" calendar>";
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
using QuantLib::Calendars::TARGET;
using QuantLib::Calendars::NewYork;
using QuantLib::Calendars::London;
using QuantLib::Calendars::Milan;
using QuantLib::Calendars::Frankfurt;
using QuantLib::Calendars::Zurich;

CalendarHandle NewTARGET()    { return CalendarHandle(new TARGET); }
CalendarHandle NewNewYork()   { return CalendarHandle(new NewYork); }
CalendarHandle NewLondon()    { return CalendarHandle(new London); }
CalendarHandle NewFrankfurt() { return CalendarHandle(new Frankfurt); }
CalendarHandle NewMilan()     { return CalendarHandle(new Milan); }
CalendarHandle NewZurich()    { return CalendarHandle(new Zurich); }
%}

%name(TARGET)    CalendarHandle NewTARGET();
%name(NewYork)   CalendarHandle NewNewYork();
%name(London)    CalendarHandle NewLondon();
%name(Frankfurt) CalendarHandle NewFrankfurt();
%name(Milan)     CalendarHandle NewMilan();
%name(Zurich)    CalendarHandle NewZurich();

// string-based factory 

%inline %{
    CalendarHandle makeCalendar(const String& name) {
        String s = StringFormatter::toLowercase(name);
        if (s == "target" || s == "euro" || s == "eur")
            return Handle<Calendar>(new TARGET);
        else if (s == "newyork" || s == "ny" || s == "nyc")
            return Handle<Calendar>(new NewYork);
        else if (s == "london" || s == "lon")
            return Handle<Calendar>(new London);
        else if (s == "milan" || s == "mil")
            return Handle<Calendar>(new Milan);
        else if (s == "frankfurt" || s == "fft")
            return Handle<Calendar>(new Frankfurt);
        else if (s == "zurich" || s == "zur")
            return Handle<Calendar>(new Zurich);
        else
            throw QuantLib::Error("Unknown calendar");
    }
%}


#endif
