
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

/*
    $Id$
    $Source$
    $Log$
    Revision 1.21  2001/05/07 10:51:34  marmar
    Error message is now more clear

    Revision 1.20  2001/05/03 15:37:07  lballabio
    Added actual/actual

    Revision 1.19  2001/04/10 07:54:33  lballabio
    Ruby histories (the Ruby way)

    Revision 1.18  2001/04/09 12:24:58  nando
    updated copyright notice header and improved CVS tags

*/

#ifndef quantlib_day_counters_i
#define quantlib_day_counters_i

%include Date.i
%include String.i

%{
using QuantLib::DayCounter;
using QuantLib::Handle;
typedef Handle<DayCounter> DayCounterHandle;
using QuantLib::DayCounters::ActualActual;
using QuantLib::DayCounters::Actual360;
using QuantLib::DayCounters::Actual365;
using QuantLib::DayCounters::Thirty360;
using QuantLib::DayCounters::Thirty360European;
using QuantLib::DayCounters::Thirty360Italian;
%}

// export Handle<DayCounter>
%name(DayCounter) class DayCounterHandle {
    %pragma(ruby) include = "Comparable";
  public:
    // constructor redefined below as string-based factory
    ~DayCounterHandle();
    // add yearFraction with default arguments
    %pragma(python) addtoclass="
    def yearFraction(self,d1,d2,startRef=None,endRef=None):
        return self.yearFractionWithRefPeriod(d1,d2,startRef,endRef)
    "
};

// replicate the DayCounter interface
%addmethods DayCounterHandle {
    #if defined(SWIGRUBY)
    void crash() {}
    #endif
    DayCounterHandle(const String& name) {
        String s = StringFormatter::toLowercase(name);
        if (s == "actact" || s == "act/act")
            return new DayCounterHandle(new ActualActual);
        else if (s == "act360" || s == "act/360")
            return new DayCounterHandle(new Actual360);
        else if (s == "act365" || s == "act/365")
            return new DayCounterHandle(new Actual365);
        else if (s == "30/360")
            return new DayCounterHandle(new Thirty360);
        else if (s == "30/360e" || s == "30/360eu")
            return new DayCounterHandle(new Thirty360European);
        else if (s == "30/360i" || s == "30/360it")
            return new DayCounterHandle(new Thirty360Italian);
        else
            throw Error("Unknown day counter");
        QL_DUMMY_RETURN(new DayCounterHandle)
    }
    int dayCount(const Date& d1, const Date& d2) {
        return (*self)->dayCount(d1,d2);
    }
    Time yearFractionWithRefPeriod(const Date& d1, const Date& d2,
        const Date& startRef, const Date& endRef) {
            return (*self)->yearFraction(d1,d2,startRef,endRef);
    }
    #if defined (SWIGPYTHON) || defined(SWIGRUBY)
    String __str__() {
        if (!self->isNull())
            return (*self)->name()+" day counter";
        else
            return "Null day counter";
    }
    int __cmp__(const DayCounterHandle& other) {
        return ((*self) == other ? 0 : 1);
    }
    #endif
    #if defined (SWIGPYTHON)
    int __nonzero__() {
        return (self->isNull() ? 0 : 1);
    }
    #endif
}

#if defined(SWIGPYTHON)
%{
DayCounterHandle NewActualActual() {
    return DayCounterHandle(new ActualActual); }
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

%name(ActualActual)         DayCounterHandle NewActualActual();
%name(Actual360)            DayCounterHandle NewActual360();
%name(Actual365)            DayCounterHandle NewActual365();
%name(Thirty360)            DayCounterHandle NewThirty360();
%name(Thirty360European)    DayCounterHandle NewThirty360European();
%name(Thirty360Italian)     DayCounterHandle NewThirty360Italian();
#endif


#endif

