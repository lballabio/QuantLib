
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
 * QuantLib license is also available at 
 * http://quantlib.sourceforge.net/LICENSE.TXT
*/

/* $Source$
   $Log$
   Revision 1.13  2001/03/09 12:40:41  lballabio
   Spring cleaning for SWIG interfaces

*/

#ifndef quantlib_day_counters_i
#define quantlib_day_counters_i

%include Date.i
%include String.i

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
    String __repr__() {
        return (*self)->name()+" day counter";
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

DayCounterHandle NewActual360() {
    return DayCounterHandle(new Actual360); }
DayCounterHandle NewActual365() { 
    return DayCounterHandle(new Actual365); }
DayCounterHandle NewThirty360() {
    return DayCounterHandle(new Thirty360); }
DayCounterHandle NewThirty360European() {
    return DayCounterHandle(new Thirty360European); }
DayCounterHandle NewThirty360Italian() {
    return DayCounterHandle(new Thirty360Italian); }
%}

%name(Actual360)            DayCounterHandle NewActual360();
%name(Actual365)            DayCounterHandle NewActual365();
%name(Thirty360)            DayCounterHandle NewThirty360();
%name(Thirty360European)    DayCounterHandle NewThirty360European();
%name(Thirty360Italian)     DayCounterHandle NewThirty360Italian();


// string-based factory 

%inline %{
    DayCounterHandle makeDayCounter(const String& name) {
        String s = StringFormatter::toLowercase(name);
        if (s == "act360" || s == "act/360")
            return DayCounterHandle(new Actual360);
        else if (s == "act365" || s == "act/365")
            return DayCounterHandle(new Actual365);
        else if (s == "30/360")
            return DayCounterHandle(new Thirty360);
        else if (s == "30/360e" || s == "30/360eu")
            return DayCounterHandle(new Thirty360European);
        else if (s == "30/360i" || s == "30/360it")
            return DayCounterHandle(new Thirty360Italian);
        else
            throw Error("Unknown currency");
        QL_DUMMY_RETURN(DayCounterHandle())
    }
%}


#endif
