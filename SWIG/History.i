
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

#ifndef quantlib_history_i
#define quantlib_history_i

%module History

%{
#include "quantlib.h"
%}

#if !defined(SWIGPYTHON)
#if !defined(PYTHON_WARNING_ISSUED)
#define PYTHON_WARNING_ISSUED
%echo "Warning: this is a Python module!!"
%echo "Exporting it to any other language is not advised"
%echo "as it could lead to unpredicted results."
#endif
#endif

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


#if defined(SWIGPYTHON)

%addmethods History {
    // element access
    double __getitem__(Date d) {
        return (*self)[d];
    }
    String __str__() {
        return "Historical data from " + 
            DateFormatter::toString(self->firstDate()) +
            " to " + DateFormatter::toString(self->lastDate());
    }
    String __repr__() {
        return "<History: historical data from " + 
            DateFormatter::toString(self->firstDate()) + 
            " to " + DateFormatter::toString(self->lastDate())+">";
    }
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
    bool __cmp__(const HistoryIterator& other) {
        return (*self == other ? 0 : -1);
    }
    String __str__() {
        return DateFormatter::toString((*self)->date()) + 
        "\t" + (IsNull((*self)->value()) ? String("Null") : 
        DoubleFormatter::toString((*self)->value()));
    }
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
    bool __cmp__(const HistoryValidIterator& other) {
        return (*self == other ? 0 : -1);
    }
    String __str__() {
        return DateFormatter::toString((*self)->date()) + 
        "\t" + (IsNull((*self)->value()) ? String("Null") : 
        DoubleFormatter::toString((*self)->value()));
    }
}

%addmethods HistoryDataIterator {
    double __float__() {
        return **self;
    }
    void advance() {
        (*self)++;
    }
    bool __cmp__(const HistoryDataIterator& other) {
        return (*self == other ? 0 : -1);
    }
    String __str__() {
        return (IsNull(**self) ? String("Null") : 
        DoubleFormatter::toString(**self));
    }
}
    
%addmethods HistoryValidDataIterator {
    double __float__() {
        return **self;
    }
    void advance() {
        (*self)++;
    }
    bool __cmp__(const HistoryValidDataIterator& other) {
        return (*self == other ? 0 : -1);
    }
    String __str__() {
        return (IsNull(**self) ? String("Null") : 
        DoubleFormatter::toString(**self));
    }
}

#endif


#endif
