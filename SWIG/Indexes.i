
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
    Revision 1.1  2001/05/16 09:57:27  lballabio
    Added indexes and piecewise flat forward curve

*/

#ifndef quantlib_indexes_i
#define quantlib_indexes_i

#if defined(SWIGPYTHON)

%include Date.i
%include Calendars.i
%include Currencies.i
%include DayCounters.i
%include TermStructures.i
%include History.i
%include String.i

%{
    typedef QuantLib::Indexes::LiborManager LiborManagerClass;
%}

class LiborManagerClass {
  public:
    void setTermStructure(CurrencyHandle currency,
        TermStructureHandle termStructure);
    TermStructureHandle getTermStructure(CurrencyHandle currency);
    bool hasTermStructure(CurrencyHandle currency);
    void setHistory(CurrencyHandle currency, int n, TimeUnit unit, 
        History termStructure);
    History getHistory(CurrencyHandle currency, int n, TimeUnit unit);
    bool hasHistory(CurrencyHandle currency, int n, TimeUnit unit);
};

%inline %{
    LiborManagerClass LiborManager;
%}


// base index class

%{
using QuantLib::Handle;
using QuantLib::Index;
typedef Handle<Index> IndexHandle;
using QuantLib::Indexes::Euribor;
using QuantLib::Indexes::Libor;
%}

// export Handle<Index>
%name(Index) class IndexHandle {
  public:
    // constructor redefined below as string-based factory
    ~IndexHandle();
};

// replicate the Calendar interface
%addmethods IndexHandle {
    IndexHandle(const String& name) {
        String s = StringFormatter::toLowercase(name);
        if (s == "euribor")
            return new IndexHandle(new Euribor);
        else if (s == "libor")
            return new IndexHandle(new Libor);
        else
            throw Error("Unknown index");
        QL_DUMMY_RETURN(new IndexHandle)
    }
    CurrencyHandle currency() {
        return (*self)->currency();
    }
    CalendarHandle calendar() {
        return (*self)->calendar();
    }
    bool modifiedFollowing() {
        return (*self)->modifiedFollowing();
    }
    DayCounterHandle dayCounter() {
        return (*self)->dayCounter();
    }
    Rate fixing(Date fixingDate, int n, TimeUnit unit) {
        return (*self)->fixing(fixingDate, n, unit);
    }
    String __str__() {
        if (!self->isNull())
            return (*self)->name()+" index";
        else
            return "Null index";
    }
}

%{
IndexHandle NewEuribor()     { return IndexHandle(new Euribor); }
IndexHandle NewLibor()       { return IndexHandle(new Libor); }
%}

%name(Euribor)    IndexHandle NewEuribor();
%name(Libor)      IndexHandle NewLibor();


#endif

#endif

