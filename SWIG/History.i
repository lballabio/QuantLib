
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
    Revision 1.16  2001/04/10 07:54:33  lballabio
    Ruby histories (the Ruby way)

    Revision 1.15  2001/04/09 15:51:16  lballabio
    Compiling again under Linux

    Revision 1.14  2001/04/09 12:24:58  nando
    updated copyright notice header and improved CVS tags

*/

#ifndef quantlib_history_i
#define quantlib_history_i

%include Date.i
%include Vectors.i
%include String.i


// History class

%{
using QuantLib::History;
typedef QuantLib::History::const_iterator HistoryIterator;
typedef QuantLib::History::const_valid_iterator HistoryValidIterator;
typedef QuantLib::History::const_data_iterator HistoryDataIterator;
typedef QuantLib::History::const_valid_data_iterator HistoryValidDataIterator;
typedef HistoryIterator HistoryEntry;
typedef HistoryValidIterator HistoryValidEntry;
using QuantLib::DateFormatter;
using QuantLib::DoubleFormatter;
%}

#if defined(SWIGPYTHON)
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
#endif

#if defined(SWIGRUBY)
class HistoryEntry {
    %pragma(ruby) include = "Comparable";
  public:
    ~HistoryEntry();
};

class HistoryValidEntry {
    %pragma(ruby) include = "Comparable";
  public:
    ~HistoryValidEntry();
};
#endif

class History {
    %pragma(ruby) include = "Enumerable";
  public:
    History(DateVector dates, DoubleVector values);
    ~History();
    #if defined(SWIGPYTHON)
    Date firstDate() const;
    Date lastDate() const;
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
    #endif
};


%addmethods History {
    #if defined(SWIGRUBY)
    void crash() {}
    #endif
    #if defined(SWIGPYTHON) || defined(SWIGRUBY)
    int __len__() {
        return self->size();
    }
    double __getitem__(Date d) {
        return (*self)[d];
    }
    String __str__() {
        return "Historical data from " +
            DateFormatter::toString(self->firstDate()) +
            " to " + DateFormatter::toString(self->lastDate());
    }
    #endif
    #if defined(SWIGRUBY)
    HistoryEntry first() {
        return self->begin();
    }
    HistoryEntry last() {
        return self->end()-1;
    }
    HistoryEntry entry(Date d) {
        QL_REQUIRE(d >= self->firstDate() && d <= self->lastDate(),
            "date outside history range");
        return self->iterator(d);
    }
    HistoryValidEntry firstValid() {
        return self->vbegin();
    }
    HistoryValidEntry lastValid() {
        return --(self->vend());
    }
    HistoryValidEntry validEntry(Date d) {
        QL_REQUIRE(d >= self->firstDate() && d <= self->lastDate(),
            "date outside history range");
        return self->valid_iterator(d);
    }
    void each() {
        for (HistoryEntry i=self->begin(); i!=self->end(); i++) {
            HistoryEntry* e = new HistoryEntry(i);
            rb_yield(Wrap_HistoryEntry(cHistoryEntry,e));
        }
    }
    #endif
}

#if defined(SWIGPYTHON)
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
    int __cmp__(const HistoryIterator& other) {
        return (*self == other ? 0 : -1);
    }
    String __str__() {
        return DateFormatter::toString((*self)->date()) +
        "\t" + ((*self)->value() == Null<double>() ? String("Null") :
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
    int __cmp__(const HistoryValidIterator& other) {
        return (*self == other ? 0 : -1);
    }
    String __str__() {
        return DateFormatter::toString((*self)->date()) +
        "\t" + ((*self)->value() == Null<double>() ? String("Null") :
        DoubleFormatter::toString((*self)->value()));
    }
}

%addmethods HistoryDataIterator {
    void advance() {
        (*self)++;
    }
    double __float__() {
        return **self;
    }
    int __cmp__(const HistoryDataIterator& other) {
        return (*self == other ? 0 : -1);
    }
    String __str__() {
        return (**self == Null<double>() ? String("Null") :
            DoubleFormatter::toString(**self));
    }
}

%addmethods HistoryValidDataIterator {
    void advance() {
        (*self)++;
    }
    double __float__() {
        return **self;
    }
    int __cmp__(const HistoryValidDataIterator& other) {
        return (*self == other ? 0 : -1);
    }
    String __str__() {
        return (**self == Null<double>() ? String("Null") :
            DoubleFormatter::toString(**self));
    }
}
#endif

#if defined(SWIGRUBY)
%addmethods HistoryEntry {
    void crash() {}
    Date date() {
        return (*self)->date();
    }
    double value() {
        return (*self)->value();
    }
    HistoryEntry succ() {
        return (*self)+1;
    }
    int __cmp__(const HistoryEntry& other) {
        return (*self == other ? 0 : (*self < other ? -1 : 1));
    }
    String __str__() {
        return DateFormatter::toString((*self)->date()) +
        "\t" + ((*self)->value() == Null<double>() ? String("Null") :
        DoubleFormatter::toString((*self)->value()));
    }
    bool isValid() {
        return (*self)->value() != Null<double>();
    }
    %pragma(ruby) pred = "isValid";
}

%addmethods HistoryValidEntry {
    void crash() {}
    Date date() {
        return (*self)->date();
    }
    double value() {
        return (*self)->value();
    }
    HistoryValidEntry succ() {
        HistoryValidEntry temp = *self;
        return ++temp;
    }
    int __cmp__(const HistoryValidEntry& other) {
        return ((*self)->date() == other->date() ? 0 : 
               ((*self)->date() < other->date() ? -1 : 1));
    }
    String __str__() {
        return DateFormatter::toString((*self)->date()) +
        "\t" + ((*self)->value() == Null<double>() ? String("Null") :
        DoubleFormatter::toString((*self)->value()));
    }
}
#endif


#endif
