
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

/*  $Source$
	$Log$
	Revision 1.19  2001/03/26 12:41:23  lballabio
	Added ruby, ruby-install and ruby-test targets to makefile (and added calendars to Ruby module in the meantime)

	Revision 1.18  2001/03/26 10:00:45  lballabio
	Turned string-based factory into Calendar constructor
	
	Revision 1.17  2001/03/26 09:59:35  lballabio
	Added Helsinki and Wellington calendars
	
	Revision 1.16  2001/03/12 17:35:11  lballabio
	Removed global IsNull function - could have caused very vicious loops
	
	Revision 1.15  2001/03/12 12:59:01  marmar
	__str__ now represents the object while __repr__ is unchanged
	
	Revision 1.14  2001/03/09 12:40:41  lballabio
	Spring cleaning for SWIG interfaces
	
*/

#ifndef quantlib_calendar_i
#define quantlib_calendar_i

%include Date.i
%include String.i

%{
using QuantLib::Handle;
using QuantLib::Calendar;
typedef Handle<Calendar> CalendarHandle;
using QuantLib::Calendars::TARGET;
using QuantLib::Calendars::NewYork;
using QuantLib::Calendars::London;
using QuantLib::Calendars::Milan;
using QuantLib::Calendars::Frankfurt;
using QuantLib::Calendars::Zurich;
using QuantLib::Calendars::Helsinki;
using QuantLib::Calendars::Wellington;
%}

// export Handle<Calendar>
%name(Calendar) class CalendarHandle {
  public:
    // constructor redefined below as string-based factory
    ~CalendarHandle();
};

// replicate the Calendar interface
%addmethods CalendarHandle {
    #if defined(SWIGRUBY)
    void crash() {}
    #endif
    CalendarHandle(const String& name) {
        String s = StringFormatter::toLowercase(name);
        if (s == "target" || s == "euro" || s == "eur")
            return new CalendarHandle(new TARGET);
        else if (s == "newyork" || s == "ny" || s == "nyc")
            return new CalendarHandle(new NewYork);
        else if (s == "london" || s == "lon")
            return new CalendarHandle(new London);
        else if (s == "milan" || s == "mil")
            return new CalendarHandle(new Milan);
        else if (s == "frankfurt" || s == "fft")
            return new CalendarHandle(new Frankfurt);
        else if (s == "zurich" || s == "zur")
            return new CalendarHandle(new Zurich);
        else if (s == "helsinki")
            return new CalendarHandle(new Helsinki);
        else if (s == "wellington")
            return new CalendarHandle(new Wellington);
        else
            throw Error("Unknown calendar");
        QL_DUMMY_RETURN(new CalendarHandle)
    }
    bool isBusinessDay(const Date& d) {
        return (*self)->isBusinessDay(d);
    }
    %pragma(ruby) pred = "isBusinessDay";
    bool isHoliday(const Date& d) {
        return (*self)->isHoliday(d);
    }
    %pragma(ruby) pred = "isHoliday";
    Date roll(const Date& d, bool modified = false) {
        return (*self)->roll(d,modified);
    }
    Date advance(const Date& d, int n, TimeUnit unit, bool modified = false) {
        return (*self)->advance(d,n,unit,modified);
    }
    #if defined (SWIGPYTHON) || defined (SWIGRUBY)
    String __str__() {
        if (!self->isNull())
            return (*self)->name()+" calendar";
        else
            return "Null calendar";
    }
    int __cmp__(const CalendarHandle& other) {
        return ((*self) == other ? 0 : 1);
    }
    #endif
    #if defined (SWIGPYTHON)
    int __nonzero__() {
        return (self->isNull() ? 0 : 1);
    }
    #endif
}

#if defined (SWIGPYTHON)
%{
CalendarHandle NewTARGET()     { return CalendarHandle(new TARGET); }
CalendarHandle NewNewYork()    { return CalendarHandle(new NewYork); }
CalendarHandle NewLondon()     { return CalendarHandle(new London); }
CalendarHandle NewFrankfurt()  { return CalendarHandle(new Frankfurt); }
CalendarHandle NewMilan()      { return CalendarHandle(new Milan); }
CalendarHandle NewZurich()     { return CalendarHandle(new Zurich); }
CalendarHandle NewHelsinki()   { return CalendarHandle(new Helsinki); }
CalendarHandle NewWellington() { return CalendarHandle(new Wellington); }
%}

%name(TARGET)     CalendarHandle NewTARGET();
%name(NewYork)    CalendarHandle NewNewYork();
%name(London)     CalendarHandle NewLondon();
%name(Frankfurt)  CalendarHandle NewFrankfurt();
%name(Milan)      CalendarHandle NewMilan();
%name(Zurich)     CalendarHandle NewZurich();
%name(Helsinki)   CalendarHandle NewHelsinki();
%name(Wellington) CalendarHandle NewWellington();
#endif


#endif

