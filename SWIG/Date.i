
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
    Revision 1.31  2001/04/23 12:29:29  lballabio
    Fixed linking in setup.py (and some tweakings in SWIG interfaces)

    Revision 1.30  2001/04/10 07:54:33  lballabio
    Ruby histories (the Ruby way)

    Revision 1.29  2001/04/09 15:51:16  lballabio
    Compiling again under Linux

    Revision 1.28  2001/04/09 12:24:58  nando
    updated copyright notice header and improved CVS tags
*/

#ifndef quantlib_date_i
#define quantlib_date_i

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

%typemap(python,in) Weekday (Weekday temp), const Weekday & (Weekday temp) {
    if (PyString_Check($source)) {
        std::string s(PyString_AsString($source));
        s = StringFormatter::toLowercase(s);
        if (s == "sun" || s == "sunday")
            temp = Sunday;
        else if (s == "mon" || s == "monday")
            temp = Monday;
        else if (s == "tue" || s == "tuesday")
            temp = Tuesday;
        else if (s == "wed" || s == "wednesday")
            temp = Wednesday;
        else if (s == "thu" || s == "thursday")
            temp = Thursday;
        else if (s == "fri" || s == "friday")
            temp = Friday;
        else if (s == "sat" || s == "saturday")
            temp = Saturday;
        else {
            PyErr_SetString(PyExc_TypeError,"not a weekday");
            return NULL;
        }
    } else {
        PyErr_SetString(PyExc_TypeError,"not a weekday");
        return NULL;
    }
    $target = &temp;
};

%typemap(python,out) Weekday, const Weekday & {
    switch (*$source) {
      case Sunday:      $target = PyString_FromString("Sunday");     break;
      case Monday:      $target = PyString_FromString("Monday");     break;
      case Tuesday:     $target = PyString_FromString("Tuesday");    break;
      case Wednesday:   $target = PyString_FromString("Wednesday");  break;
      case Thursday:    $target = PyString_FromString("Thursday");   break;
      case Friday:      $target = PyString_FromString("Friday");     break;
      case Saturday:    $target = PyString_FromString("Saturday");   break;
    }
};

%typemap(python,ret) Weekday {
    delete $source;
}


%typemap(ruby,in) Weekday (Weekday temp), const Weekday & (Weekday temp) {
    if (TYPE($source) == T_STRING) {
        std::string s(STR2CSTR($source));
        s = StringFormatter::toLowercase(s);
        if (s == "sun" || s == "sunday")
            temp = Sunday;
        else if (s == "mon" || s == "monday")
            temp = Monday;
        else if (s == "tue" || s == "tuesday")
            temp = Tuesday;
        else if (s == "wed" || s == "wednesday")
            temp = Wednesday;
        else if (s == "thu" || s == "thursday")
            temp = Thursday;
        else if (s == "fri" || s == "friday")
            temp = Friday;
        else if (s == "sat" || s == "saturday")
            temp = Saturday;
        else
            rb_raise(rb_eTypeError,"not a weekday");
    } else {
        rb_raise(rb_eTypeError,"not a weekday");
    }
    $target = &temp;
};

%typemap(ruby,out) Weekday, const Weekday & {
    switch (*$source) {
      case Sunday:      $target = rb_str_new2("Sunday");     break;
      case Monday:      $target = rb_str_new2("Monday");     break;
      case Tuesday:     $target = rb_str_new2("Tuesday");    break;
      case Wednesday:   $target = rb_str_new2("Wednesday");  break;
      case Thursday:    $target = rb_str_new2("Thursday");   break;
      case Friday:      $target = rb_str_new2("Friday");     break;
      case Saturday:    $target = rb_str_new2("Saturday");   break;
    }
};

%typemap(ruby,ret) Weekday {
    delete $source;
};



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

%typemap(python,in) Month (Month temp), const Month & (Month temp) {
    if (PyString_Check($source)) {
        std::string s(PyString_AsString($source));
        s = StringFormatter::toLowercase(s);
        if (s == "jan" || s == "january")
            temp = January;
        else if (s == "feb" || s == "february")
            temp = February;
        else if (s == "mar" || s == "march")
            temp = March;
        else if (s == "apr" || s == "april")
            temp = April;
        else if (s == "may")
            temp = May;
        else if (s == "jun" || s == "june")
            temp = June;
        else if (s == "jul" || s == "july")
            temp = July;
        else if (s == "aug" || s == "august")
            temp = August;
        else if (s == "sep" || s == "september")
            temp = September;
        else if (s == "oct" || s == "october")
            temp = October;
        else if (s == "nov" || s == "november")
            temp = November;
        else if (s == "dec" || s == "december")
            temp = December;
        else {
            PyErr_SetString(PyExc_TypeError,"not a month");
            return NULL;
        }
    } else if (PyInt_Check($source)) {
        int i = int(PyInt_AsLong($source));
        if (i>=1 && i<=12)
            temp = Month(i);
        else {
            PyErr_SetString(PyExc_TypeError,"not a month");
            return NULL;
        }
    } else {
        PyErr_SetString(PyExc_TypeError,"not a weekday");
        return NULL;
    }
    $target = &temp;
};

%typemap(python,out) Month, const Month & {
    switch (*$source) {
      case January:     $target = PyString_FromString("January");    break;
      case February:    $target = PyString_FromString("February");   break;
      case March:       $target = PyString_FromString("March");      break;
      case April:       $target = PyString_FromString("April");      break;
      case May:         $target = PyString_FromString("May");        break;
      case June:        $target = PyString_FromString("June");       break;
      case July:        $target = PyString_FromString("July");       break;
      case August:      $target = PyString_FromString("August");     break;
      case September:   $target = PyString_FromString("September");  break;
      case October:     $target = PyString_FromString("October");    break;
      case November:    $target = PyString_FromString("November");   break;
      case December:    $target = PyString_FromString("December");   break;
    }
};

%typemap(python,ret) Month {
    delete $source;
}


%typemap(ruby,in) Month (Month temp), const Month & (Month temp) {
    if (TYPE($source) == T_STRING) {
        std::string s(STR2CSTR($source));
        s = StringFormatter::toLowercase(s);
        if (s == "jan" || s == "january")
            temp = January;
        else if (s == "feb" || s == "february")
            temp = February;
        else if (s == "mar" || s == "march")
            temp = March;
        else if (s == "apr" || s == "april")
            temp = April;
	else if (s == "may")
            temp = May;
        else if (s == "jun" || s == "june")
            temp = June;
        else if (s == "jul" || s == "july")
            temp = July;
        else if (s == "aug" || s == "august")
            temp = August;
        else if (s == "sep" || s == "september")
            temp = September;
        else if (s == "oct" || s == "october")
            temp = October;
        else if (s == "nov" || s == "november")
            temp = November;
        else if (s == "dec" || s == "december")
            temp = December;
        else
            rb_raise(rb_eTypeError,"not a month");
    } else if (TYPE($source) == T_FIXNUM) {
        int i = NUM2INT($source);
        if (i>=1 && i<=12)
            temp = Month(i);
        else
            rb_raise(rb_eTypeError,"not a month");
    } else {
        rb_raise(rb_eTypeError,"not a month");
    }
    $target = &temp;
};

%typemap(ruby,out) Month, const Month & {
    switch (*$source) {
      case January:     $target = rb_str_new2("January");    break;
      case February:    $target = rb_str_new2("February");   break;
      case March:       $target = rb_str_new2("March");      break;
      case April:       $target = rb_str_new2("April");      break;
      case May:         $target = rb_str_new2("May");        break;
      case June:        $target = rb_str_new2("June");       break;
      case July:        $target = rb_str_new2("July");       break;
      case August:      $target = rb_str_new2("August");     break;
      case September:   $target = rb_str_new2("September");  break;
      case October:     $target = rb_str_new2("October");    break;
      case November:    $target = rb_str_new2("November");   break;
      case December:    $target = rb_str_new2("December");   break;
    }
};

%typemap(ruby,ret) Month {
    delete $source;
};

// typemap time units to corresponding strings

%{
using QuantLib::TimeUnit;
using QuantLib::Days;
using QuantLib::Weeks;
using QuantLib::Months;
using QuantLib::Years;
%}

%typemap(python,in) TimeUnit (TimeUnit temp), const TimeUnit& (TimeUnit temp) {
    if (PyString_Check($source)) {
        std::string s(PyString_AsString($source));
        s = StringFormatter::toLowercase(s);
        if (s == "d" || s == "day" || s == "days")
            temp = Days;
        else if (s == "w" || s == "week" || s == "weeks")
            temp = Weeks;
        else if (s == "m" || s == "month" || s == "months")
            temp = Months;
        else if (s == "y" || s == "year" || s == "years")
            temp = Years;
        else {
            PyErr_SetString(PyExc_TypeError,"not a time unit");
            return NULL;
        }
    } else {
        PyErr_SetString(PyExc_TypeError,"not a time unit");
        return NULL;
    }
    $target = &temp;
};

%typemap(python,out) TimeUnit, const TimeUnit & {
    switch (*$source) {
      case Days:    $target = PyString_FromString("days");   break;
      case Weeks:   $target = PyString_FromString("weeks");  break;
      case Months:  $target = PyString_FromString("months"); break;
      case Years:   $target = PyString_FromString("years");  break;
    }
};

%typemap(python,ret) TimeUnit {
    delete $source;
}


%typemap(ruby,in) TimeUnit (TimeUnit temp), const TimeUnit & (TimeUnit temp) {
    if (TYPE($source) == T_STRING) {
        std::string s(STR2CSTR($source));
        s = StringFormatter::toLowercase(s);
        if (s == "d" || s == "day" || s == "days")
            temp = Days;
        else if (s == "w" || s == "week" || s == "weeks")
            temp = Weeks;
        else if (s == "m" || s == "month" || s == "months")
            temp = Months;
        else if (s == "y" || s == "year" || s == "years")
            temp = Years;
        else
            rb_raise(rb_eTypeError,"not a time unit");
    } else {
        rb_raise(rb_eTypeError,"not a time unit");
    }
    $target = &temp;
};

%typemap(ruby,out) TimeUnit, const TimeUnit & {
    switch (*$source) {
      case Days:    $target = rb_str_new2("days");   break;
      case Weeks:   $target = rb_str_new2("weeks");  break;
      case Months:  $target = rb_str_new2("months"); break;
      case Years:   $target = rb_str_new2("years");  break;
    }
};

%typemap(ruby,ret) TimeUnit {
    delete $source;
}

// and finally, the Date class

class Date {
    %pragma(ruby) include = "Comparable";
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

    #if defined(SWIGRUBY)
    void crash() {}
    #endif

    int monthNumber() {
        return int(self->month());
    }
    int weekdayNumber() {
        return int(self->weekday());
    }

    #if defined(SWIGPYTHON) || defined(SWIGRUBY)
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
    #endif

    #if defined(SWIGPYTHON)
    Date __iadd__(int days) {
        return self->plusDays(days);
    }
    Date __isub__(int days) {
        return self->plusDays(-days);
    }
    int __nonzero__() {
        return (*self == Date() ? 0 : 1);
    }
    #endif

    #if defined(SWIGRUBY)
    Date succ() {
        return self->plusDays(1);
    }
    #endif
}

// typemap None to null Date

%typemap(python,in) Date (Date temp), Date * (Date temp),
  const Date & (Date temp), Date & (Date temp) {
    Date* x;
    if ($source == Py_None) {
        temp = Null<Date>();
        $target = &temp;
    } else if ((SWIG_ConvertPtr($source,(void **) &x,
      (swig_type_info *)SWIG_TypeQuery("Date *"),0)) != -1) {
        $target = x;
    } else {
        PyErr_SetString(PyExc_TypeError,"Date expected");
        return NULL;
    }
};

%{
typedef std::vector<Date> DateVector;
using QuantLib::Null;
%}

class DateVector {
    %pragma(ruby) include = "Enumerable";
  public:
    ~DateVector();
};

%typemap(ruby,in) VALUE {
    $target = $source;
};

%addmethods DateVector {

    #if defined(SWIGRUBY)
    void crash() {}
    DateVector(VALUE v) {
    	if (rb_obj_is_kind_of(v,rb_cArray)) {
            int size = RARRAY(v)->len;
            DateVector* temp = new DateVector(size);
            for (int i=0; i<size; i++) {
                VALUE o = RARRAY(v)->ptr[i];
                if (o == Qnil) {
                    (*temp)[i] = Date();
                } else {
                    Date* d;
                    Get_Date(o,d);
                    (*temp)[i] = *d;
                }
            }
            return temp;
        } else {
            rb_raise(rb_eTypeError,
                "wrong argument type (expected array)");
        }
    }
    #endif
    #if defined(SWIGPYTHON)
    DateVector(const DateVector& v) {
        return new DateVector(v);
    }
    #endif
    #if defined(SWIGPYTHON) || defined(SWIGRUBY)
    String __str__() {
        String s = "(";
        for (int i=0; i<self->size(); i++) {
            if (i != 0)
                s += ", ";
            s += DateFormatter::toString((*self)[i]);
        }
        s += ")";
        return s;
    }
    int __len__() {
        return self->size();
    }
    Date __getitem__(int i) {
        if (i>=0 && i<self->size()) {
            return (*self)[i];
        } else if (i<0 && -i<=self->size()) {
            return (*self)[self->size()+i];
        } else {
            throw IndexError("DateVector index out of range");
        }
        QL_DUMMY_RETURN(Date())
    }
    void __setitem__(int i, const Date& x) {
        if (i>=0 && i<self->size()) {
            (*self)[i] = x;
        } else if (i<0 && -i<=self->size()) {
            (*self)[self->size()+i] = x;
        } else {
            throw IndexError("DateVector index out of range");
        }
    }
    #endif
    #if defined(SWIGPYTHON)
    DateVector __getslice__(int i, int j) {
        if (i<0)
            i = self->size()+i;
        if (i<0)
            i = 0;
        if (j<0)
            j = self->size()+j;
        if (j > self->size())
            j = self->size();
        DateVector tmp(j-i);
        std::copy(self->begin()+i,self->begin()+j,tmp.begin());
        return tmp;
    }
    void __setslice__(int i, int j, const DateVector& rhs) {
        if (i<0)
            i = self->size()+i;
        if (i<0)
            i = 0;
        if (j<0)
            j = self->size()+j;
        if (j > self->size())
            j = self->size();
        QL_ENSURE(rhs.size() == j-i, "DateVectors are not resizable");
        std::copy(rhs.begin(),rhs.end(),self->begin()+i);
    }
    int __nonzero__() {
        return (self->size() == 0 ? 0 : 1);
    }
    #endif
    #if defined(SWIGRUBY)
    void each() {
        for (int i=0; i<self->size(); i++) {
            Date* d = new Date((*self)[i]);
            rb_yield(Wrap_Date(cDate,d));
        }
    }
    #endif
}; 

%typemap(python,in) DateVector (DateVector temp),
  DateVector * (DateVector temp), const DateVector & (DateVector temp),
  DateVector & (DateVector temp) {
    DateVector* v;
    if (PyTuple_Check($source) || PyList_Check($source)) {
        int size = (PyTuple_Check($source) ?
            PyTuple_Size($source) :
            PyList_Size($source));
        temp = DateVector(size);
        $target = &temp;
        for (int i=0; i<size; i++) {
            Date* x;
            PyObject* o = PySequence_GetItem($source,i);
            if (o == Py_None) {
                (*$target)[i] = Null<Date>();
            } else if ((SWIG_ConvertPtr(o,(void **) &x,
              (swig_type_info *)SWIG_TypeQuery("Date *"),0)) != -1) {
                (*$target)[i] = *x;
            } else {
                PyErr_SetString(PyExc_TypeError,"Dates expected");
                return NULL;
            }
        }
    } else if ((SWIG_ConvertPtr($source,(void **) &v,
      (swig_type_info *)SWIG_TypeQuery("DateVector *"),0)) != -1) {
        $target = v;
    } else {
        PyErr_SetString(PyExc_TypeError,"DateVector expected");
        return NULL;
    }
};

%inline %{
    Date DateFromSerialNumber(int serialNumber) {
        return Date(serialNumber);
    }
%}


#endif

