
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

#ifndef quantlib_date_i
#define quantlib_date_i

%module Date

%{
#include "quantlib.h"
%}

#if !defined(SWIGPYTHON)
#if !defined(PYTHON_WARNING_ISSUED)
#define PYTHON_WARNING_ISSUED
%echo "Warning: Date is a Python module!!"
%echo "Exporting it to any other language is not advised"
%echo "as it could lead to unpredicted results."
#endif
#endif

%include String.i

%{
#include <cstdlib>
#include <string>
using QuantLib::Date;

using QuantLib::Day;
using QuantLib::Year;
using QuantLib::Time;
%}

typedef int Day;
typedef int Year;
typedef double Time;

// typemap weekdays to corresponding strings

%{
using QuantLib::Weekday;
using QuantLib::Sunday;
using QuantLib::Monday;
using QuantLib::Tuesday;
using QuantLib::Wednesday;
using QuantLib::Thursday;
using QuantLib::Friday;
using QuantLib::Saturday;

using QuantLib::StringFormatter;
using QuantLib::DateFormatter;
%}

%typemap(python,in) Weekday, const Weekday & {
    if (PyString_Check($source)) {
        std::string s(PyString_AsString($source));
        s = StringFormatter::toLowercase(s);
        if (s == "sun" || s == "sunday")
            $target = new Weekday(Sunday);
        else if (s == "mon" || s == "monday")
            $target = new Weekday(Monday);
        else if (s == "tue" || s == "tuesday")
            $target = new Weekday(Tuesday);
        else if (s == "wed" || s == "wednesday")
            $target = new Weekday(Wednesday);
        else if (s == "thu" || s == "thursday")
            $target = new Weekday(Thursday);
        else if (s == "fri" || s == "friday")
            $target = new Weekday(Friday);
        else if (s == "sat" || s == "saturday")
            $target = new Weekday(Saturday);
        else {
            PyErr_SetString(PyExc_TypeError,"not a weekday");
            return NULL;
        }
    } else {
        PyErr_SetString(PyExc_TypeError,"not a weekday");
        return NULL;
    }
};

%typemap(python,freearg) Weekday, const Weekday & {
    delete $source;
};

%typemap(python,out) Weekday, const Weekday & {
    switch (*$source) {
      case Sunday:        $target = PyString_FromString("Sunday");    break;
      case Monday:        $target = PyString_FromString("Monday");    break;
      case Tuesday:        $target = PyString_FromString("Tuesday");    break;
      case Wednesday:    $target = PyString_FromString("Wednesday");    break;
      case Thursday:    $target = PyString_FromString("Thursday");    break;
      case Friday:        $target = PyString_FromString("Friday");    break;
      case Saturday:    $target = PyString_FromString("Saturday");    break;
    }
};

%typemap(python,ret) Weekday {
    delete $source;
}


// typemap months to corresponding strings or numbers

%{
using QuantLib::Month;
using QuantLib::January;
using QuantLib::February;
using QuantLib::March;
using QuantLib::April;
using QuantLib::May;
using QuantLib::June;
using QuantLib::July;
using QuantLib::August;
using QuantLib::September;
using QuantLib::October;
using QuantLib::November;
using QuantLib::December;
%}

%typemap(python,in) Month, const Month & {
    if (PyString_Check($source)) {
        std::string s(PyString_AsString($source));
        s = StringFormatter::toLowercase(s);
        if (s == "jan" || s == "january")
            $target = new Month(January);
        else if (s == "feb" || s == "february")
            $target = new Month(February);
        else if (s == "mar" || s == "march")
            $target = new Month(March);
        else if (s == "apr" || s == "april")
            $target = new Month(April);
        else if (s == "may")
            $target = new Month(May);
        else if (s == "jun" || s == "june")
            $target = new Month(June);
        else if (s == "jul" || s == "july")
            $target = new Month(July);
        else if (s == "aug" || s == "august")
            $target = new Month(August);
        else if (s == "sep" || s == "september")
            $target = new Month(September);
        else if (s == "oct" || s == "october")
            $target = new Month(October);
        else if (s == "nov" || s == "november")
            $target = new Month(November);
        else if (s == "dec" || s == "december")
            $target = new Month(December);
        else {
            PyErr_SetString(PyExc_TypeError,"not a month");
            return NULL;
        }
    } else if (PyInt_Check($source)) {
        int i = int(PyInt_AsLong($source));
        if (i>=1 && i<=12)
            $target = new Month(Month(i));
        else {
            PyErr_SetString(PyExc_TypeError,"not a month");
            return NULL;
        }
    } else {
        PyErr_SetString(PyExc_TypeError,"not a weekday");
        return NULL;
    }
};

%typemap(python,freearg) Month, const Month & {
    delete $source;
};

%typemap(python,out) Month, const Month & {
    switch (*$source) {
      case January:        $target = PyString_FromString("January");    break;
      case February:    $target = PyString_FromString("February");    break;
      case March:        $target = PyString_FromString("March");        break;
      case April:        $target = PyString_FromString("April");        break;
      case May:            $target = PyString_FromString("May");        break;
      case June:        $target = PyString_FromString("June");        break;
      case July:        $target = PyString_FromString("July");        break;
      case August:        $target = PyString_FromString("August");    break;
      case September:    $target = PyString_FromString("September");    break;
      case October:        $target = PyString_FromString("October");    break;
      case November:    $target = PyString_FromString("November");    break;
      case December:    $target = PyString_FromString("December");    break;
    }
};

%typemap(python,ret) Month {
    delete $source;
}


// typemap time units to corresponding strings

%{
using QuantLib::TimeUnit;
using QuantLib::Days;
using QuantLib::Weeks;
using QuantLib::Months;
using QuantLib::Years;
%}

%typemap(python,in) TimeUnit, const TimeUnit & {
    if (PyString_Check($source)) {
        std::string s(PyString_AsString($source));
        s = StringFormatter::toLowercase(s);
        if (s == "d" || s == "day" || s == "days")
            $target = new TimeUnit(Days);
        else if (s == "w" || s == "week" || s == "weeks")
            $target = new TimeUnit(Weeks);
        else if (s == "m" || s == "month" || s == "months")
            $target = new TimeUnit(Months);
        else if (s == "y" || s == "year" || s == "years")
            $target = new TimeUnit(Years);
        else {
            PyErr_SetString(PyExc_TypeError,"unknown time unit");
            return NULL;
        }
    } else {
        PyErr_SetString(PyExc_TypeError,"not a time unit");
        return NULL;
    }
};

%typemap(python,freearg) TimeUnit, const TimeUnit & {
    delete $source;
};

%typemap(python,out) TimeUnit, const TimeUnit & {
    switch (*$source) {
      case Days:
        $target = PyString_FromString("days"); break;
      case Weeks:
        $target = PyString_FromString("weeks"); break;
      case Months:
        $target = PyString_FromString("months"); break;
      case Years:
        $target = PyString_FromString("years"); break;
    }
};

%typemap(python,ret) TimeUnit {
    delete $source;
}


// and finally, the Date class

class Date {
  public:
    Date(Day d, Month m, Year y);
    ~Date();
    // access functions
    Weekday weekday() const;
    Day dayOfMonth() const;
    Day dayOfYear() const;        // one-based
    Month month() const;
    Year year() const;
    int serialNumber() const;
    // increment/decrement dates
    Date plusDays(int days) const;
    Date plusWeeks(int weeks) const;
    Date plusMonths(int months) const;
    Date plusYears(int years) const;
    Date plus(int units, TimeUnit) const;
    // leap years
    static bool isLeap(Year y);
    // earliest and latest allowed date
    static Date minDate();
    static Date maxDate();
};

%addmethods Date {
    int monthNumber() {
        return int(self->month());
    }
    int weekdayNumber() {
        return int(self->weekday());
    }
}
    
#if defined(SWIGPYTHON)

%addmethods Date {
    Date __add__(int days) {
        return self->plusDays(days);
    }
    Date __sub__(int days) {
        return self->plusDays(-days);
    }
    int __cmp__(const Date& other) {
        if (*self < other)
            return -1;
        if (*self == other)
            return 0;
        return 1;
    }
    String __str__() {
        return DateFormatter::toString(*self);
    }
    String __repr__() {
        return "<Date: "+DateFormatter::toString(*self)+">";
    }
}

// typemap Python list of dates to std::vector<Date>

%{
typedef std::vector<Date> DateVector;
%}

class DateVector {
  public:
    // hide constructor - python sequences can be used
    ~DateVector();
};

%addmethods DateVector {
    int __len__() {
        return self->size();
    }
    Date __getitem__(int i) {
        if (i>=0 && i<self->size()) {
            return (*self)[i];
        } else if (i<0 && -i<=self->size()) {
            return (*self)[self->size()+i];
        } else {
            throw std::domain_error("DateVector");
        }
    }
    void __setitem__(int i, Date d) {
        (*self)[i] = d;
    }
    String __str__() {
        String s = "(";
        for (int i=0; i<self->size(); i++) {
            if (i != 0)
                s += ", ";
            s += QuantLib::DateFormatter::toString((*self)[i]);
        }
        s += ")";
        return s;
    }
}; 

%typemap(python,in) DateVector (bool newObj), DateVector * (bool newObj), 
  const DateVector & (bool newObj), DateVector & (bool newObj) {
    DateVector* v;
    if ((SWIG_ConvertPtr($source,(void **) &v,
      (swig_type_info *)SWIG_TypeQuery("DateVector *"),0)) != -1) {
        $target = v;
        newObj = false;
    } else if (PyTuple_Check($source)) {
        int size = PyTuple_Size($source);
        $target = new DateVector(size);
        newObj = true;
        for (int i=0; i<size; i++) {
            Date* d;
            PyObject* o = PyTuple_GetItem($source,i);
            if ((SWIG_ConvertPtr(o,(void **) &d,
              (swig_type_info *)SWIG_TypeQuery("Date *"),0)) != -1) {
                (*$target)[i] = *d;
            } else {
                PyErr_SetString(PyExc_TypeError,"tuple must contain dates");
                delete $target;
                return NULL;
            }
        }
    } else if (PyList_Check($source)) {
        int size = PyList_Size($source);
        $target = new DateVector(size);
        newObj = true;
        for (int i=0; i<size; i++) {
            Date* d;
            PyObject* o = PyList_GetItem($source,i);
            if ((SWIG_ConvertPtr(o,(void **) &d,
              (swig_type_info *)SWIG_TypeQuery("Date *"),0)) != -1) {
                (*$target)[i] = *d;
            } else {
                PyErr_SetString(PyExc_TypeError,"list must contain dates");
                delete $target;
                return NULL;
            }
        }
    } else {
        PyErr_SetString(PyExc_TypeError,"not a sequence");
        return NULL;
    }
};

%typemap(python,freearg) DateVector, DateVector *, const DateVector &, 
  DateVector & {
    if (newObj)
        delete $source;
};


#endif


%inline %{
    Date DateFromSerialNumber(int serialNumber) {
        return Date(serialNumber);
    }
%}


#endif
