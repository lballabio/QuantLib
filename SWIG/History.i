
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

/* $Source$
   $Log$
   Revision 1.9  2001/03/12 12:59:01  marmar
   __str__ now represents the object while __repr__ is unchanged

   Revision 1.8  2001/03/09 12:40:41  lballabio
   Spring cleaning for SWIG interfaces

*/

#ifndef quantlib_history_i
#define quantlib_history_i

%include Date.i
%include Vectors.i
%include String.i

%{
#include "history.h"
%}

// History class

%{
using QuantLib::History;
typedef QuantLib::History::const_iterator HistoryIterator;
typedef QuantLib::History::const_valid_iterator HistoryValidIterator;
typedef QuantLib::History::const_data_iterator HistoryDataIterator;
typedef QuantLib::History::const_valid_data_iterator HistoryValidDataIterator;
using QuantLib::DateFormatter;
using QuantLib::DoubleFormatter;
%}

class HistoryIterator {
  public:
    ~HistoryIterator();
};

class HistoryValidIterator {
  public:
    ~HistoryValidIterator();
};

class HistoryDataIterator {
  public:
    ~HistoryDataIterator();
};

class HistoryValidDataIterator {
  public:
    ~HistoryValidDataIterator();
};

class History {
  public:
    History(DateVector dates, DoubleVector values);
    ~History();
    Date firstDate() const;
    Date lastDate() const;
    int size() const;
    HistoryIterator begin() const;
    HistoryIterator end() const;
    HistoryIterator iterator(Date d) const;
    HistoryValidIterator vbegin() const;
    HistoryValidIterator vend() const;
    HistoryValidIterator valid_iterator(Date d) const;
    HistoryDataIterator dbegin() const;
    HistoryDataIterator dend() const;
    HistoryDataIterator data_iterator(Date d) const;
    HistoryValidDataIterator vdbegin() const;
    HistoryValidDataIterator vdend() const;
    HistoryValidDataIterator valid_data_iterator(Date d) const;
};


%addmethods History {
    #if defined(SWIGPYTHON)
    double __getitem__(Date d) {
        return (*self)[d];
    }
    String __str__() {
        return "Historical data from " + 
            DateFormatter::toString(self->firstDate()) +
            " to " + DateFormatter::toString(self->lastDate());
    }
    #endif
}

%addmethods HistoryIterator {
    Date date() {
        return (*self)->date();
    }
    double value() {
        return (*self)->value();
    }
    void advance() {
        (*self)++;
    }
    #if defined(SWIGPYTHON)
    int __cmp__(const HistoryIterator& other) {
        return (*self == other ? 0 : -1);
    }
    String __str__() {
        return DateFormatter::toString((*self)->date()) + 
        "\t" + (IsNull((*self)->value()) ? String("Null") : 
        DoubleFormatter::toString((*self)->value()));
    }
    #endif
}

%addmethods HistoryValidIterator {
    Date date() {
        return (*self)->date();
    }
    double value() {
        return (*self)->value();
    }
    void advance() {
        (*self)++;
    }
    #if defined(SWIGPYTHON)
    int __cmp__(const HistoryValidIterator& other) {
        return (*self == other ? 0 : -1);
    }
    String __str__() {
        return DateFormatter::toString((*self)->date()) + 
        "\t" + (IsNull((*self)->value()) ? String("Null") : 
        DoubleFormatter::toString((*self)->value()));
    }
    #endif
}

%addmethods HistoryDataIterator {
    void advance() {
        (*self)++;
    }
    #if defined(SWIGPYTHON)
    double __float__() {
        return **self;
    }
    int __cmp__(const HistoryDataIterator& other) {
        return (*self == other ? 0 : -1);
    }
    String __str__() {
        return (IsNull(**self) ? String("Null") : 
        DoubleFormatter::toString(**self));
    }
    #endif
}
    
%addmethods HistoryValidDataIterator {
    void advance() {
        (*self)++;
    }
    #if defined(SWIGPYTHON)
    double __float__() {
        return **self;
    }
    int __cmp__(const HistoryValidDataIterator& other) {
        return (*self == other ? 0 : -1);
    }
    String __str__() {
        return (IsNull(**self) ? String("Null") : 
        DoubleFormatter::toString(**self));
    }
    #endif
}


#endif
