
/* 
Copyright (C) 2000 F.Ametrano, L.Ballabio, A.Benin, M.Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

#ifndef quantlib_date_i
#define quantlib_date_i

%module Date

#if !defined(SWIGPYTHON)
#if !defined(PYTHON_WARNING_ISSUED)
#define PYTHON_WARNING_ISSUED
%echo "Warning: this is a Python module!!"
%echo "Exporting it to any other language is not advised as it could lead to unpredicted results."
#endif
#endif

%{
#include <cstdlib>
#include <string>
#include "date.h"
QL_USING(QuantLib,Date)
%}

// typedefs

%{
QL_USING(QuantLib,Day)
QL_USING(QuantLib,Year)
QL_USING(QuantLib,Time)
%}

typedef int Day;
typedef int Year;
typedef double Time;

// typemap weekdays to corresponding strings

%{
QL_USING(QuantLib,Weekday)
QL_USING(QuantLib,Sunday)
QL_USING(QuantLib,Monday)
QL_USING(QuantLib,Tuesday)
QL_USING(QuantLib,Wednesday)
QL_USING(QuantLib,Thursday)
QL_USING(QuantLib,Friday)
QL_USING(QuantLib,Saturday)

#include "stringconverters.h"
QL_USING(QuantLib,ConvertToLowercase)
%}

%typemap(python,in) Weekday, Weekday * {
	if (PyString_Check($source)) {
		std::string s(PyString_AsString($source));
		ConvertToLowercase(s);
		if (s == "sun" || s == "sunday")			$target = new Weekday(Sunday);
		else if (s == "mon" || s == "monday")		$target = new Weekday(Monday);
		else if (s == "tue" || s == "tuesday")		$target = new Weekday(Tuesday);
		else if (s == "wed" || s == "wednesday")	$target = new Weekday(Wednesday);
		else if (s == "thu" || s == "thursday")		$target = new Weekday(Thursday);
		else if (s == "fri" || s == "friday")		$target = new Weekday(Friday);
		else if (s == "sat" || s == "saturday")		$target = new Weekday(Saturday);
		else {
			PyErr_SetString(PyExc_TypeError,"not a weekday");
			return NULL;
		}
	} else {
		PyErr_SetString(PyExc_TypeError,"not a weekday");
		return NULL;
	}
};

%typemap(python,freearg) Weekday, Weekday * {
	delete $source;
};

%typemap(python,out) Weekday, Weekday * {
	switch (*$source) {
	  case Sunday:		$target = PyString_FromString("Sunday");	break;
	  case Monday:		$target = PyString_FromString("Monday");	break;
	  case Tuesday:		$target = PyString_FromString("Tuesday");	break;
	  case Wednesday:	$target = PyString_FromString("Wednesday");	break;
	  case Thursday:	$target = PyString_FromString("Thursday");	break;
	  case Friday:		$target = PyString_FromString("Friday");	break;
	  case Saturday:	$target = PyString_FromString("Saturday");	break;
	}
};

%typemap(python,ret) Weekday {
	delete $source;
}


// typemap months to corresponding strings or numbers

%{
QL_USING(QuantLib,Month)
QL_USING(QuantLib,January)
QL_USING(QuantLib,February)
QL_USING(QuantLib,March)
QL_USING(QuantLib,April)
QL_USING(QuantLib,May)
QL_USING(QuantLib,June)
QL_USING(QuantLib,July)
QL_USING(QuantLib,August)
QL_USING(QuantLib,September)
QL_USING(QuantLib,October)
QL_USING(QuantLib,November)
QL_USING(QuantLib,December)
%}

%typemap(python,in) Month, Month * {
	if (PyString_Check($source)) {
		std::string s(PyString_AsString($source));
		ConvertToLowercase(s);
		if (s == "jan" || s == "january")			$target = new Month(January);
		else if (s == "feb" || s == "february")		$target = new Month(February);
		else if (s == "mar" || s == "march")		$target = new Month(March);
		else if (s == "apr" || s == "april")		$target = new Month(April);
		else if (s == "may")						$target = new Month(May);
		else if (s == "jun" || s == "june")			$target = new Month(June);
		else if (s == "jul" || s == "july")			$target = new Month(July);
		else if (s == "aug" || s == "august")		$target = new Month(August);
		else if (s == "sep" || s == "september")	$target = new Month(September);
		else if (s == "oct" || s == "october")		$target = new Month(October);
		else if (s == "nov" || s == "november")		$target = new Month(November);
		else if (s == "dec" || s == "december")		$target = new Month(December);
		else {
			PyErr_SetString(PyExc_TypeError,"not a month");
			return NULL;
		}
	} else if (PyInt_Check($source)) {
		int i = int(PyInt_AsLong($source));
		if (i>=1 && i<=12)
			$target = new Month(Month(i-1));
		else {
			PyErr_SetString(PyExc_TypeError,"not a month");
			return NULL;
		}
	} else {
		PyErr_SetString(PyExc_TypeError,"not a weekday");
		return NULL;
	}
};

%typemap(python,freearg) Month, Month * {
	delete $source;
};

%typemap(python,out) Month, Month * {
	switch (*$source) {
	  case January:		$target = PyString_FromString("January");	break;
	  case February:	$target = PyString_FromString("February");	break;
	  case March:		$target = PyString_FromString("March");		break;
	  case April:		$target = PyString_FromString("April");		break;
	  case May:			$target = PyString_FromString("May");		break;
	  case June:		$target = PyString_FromString("June");		break;
	  case July:		$target = PyString_FromString("July");		break;
	  case August:		$target = PyString_FromString("August");	break;
	  case September:	$target = PyString_FromString("September");	break;
	  case October:		$target = PyString_FromString("October");	break;
	  case November:	$target = PyString_FromString("November");	break;
	  case December:	$target = PyString_FromString("December");	break;
	}
};

%typemap(python,ret) Month {
	delete $source;
}


// typemap time units to corresponding strings

%{
	QL_USING(QuantLib,TimeUnit)
	QL_USING(QuantLib,Days)
	QL_USING(QuantLib,Weeks)
	QL_USING(QuantLib,Months)
	QL_USING(QuantLib,Years)
%}

%typemap(python,in) TimeUnit, TimeUnit * {
	if (PyString_Check($source)) {
		std::string s(PyString_AsString($source));
		ConvertToLowercase(s);
		if (s == "d" || s == "day" || s == "days")			$target = new TimeUnit(Days);
		else if (s == "w" || s == "week" || s == "weeks")	$target = new TimeUnit(Weeks);
		else if (s == "m" || s == "month" || s == "months")	$target = new TimeUnit(Months);
		else if (s == "y" || s == "year" || s == "years")	$target = new TimeUnit(Years);
		else {
			PyErr_SetString(PyExc_TypeError,"unknown time unit");
			return NULL;
		}
	} else {
		PyErr_SetString(PyExc_TypeError,"not a time unit");
		return NULL;
	}
};

%typemap(python,freearg) TimeUnit, TimeUnit * {
	delete $source;
};

%typemap(python,out) TimeUnit, TimeUnit * {
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
	Weekday dayOfWeek() const;
	Day dayOfMonth() const;
	Day dayOfYear() const;		// zero-based
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

#if defined(SWIGPYTHON)

%{
#include "formats.h"
QL_USING(QuantLib,DateFormat)
%}

%addmethods Date {
	Date __add__(int days) {
		return self->plusDays(days);
	}
	Date __sub__(int days) {
		return self->plusDays(-days);
	}
	char* __str__() {
		static char temp[256];
		strcpy(temp,DateFormat(*self).c_str());
		return temp;
	}
	char* __repr__() {
		static char temp[256];
		std::string s = "<Date: "+DateFormat(*self)+">";
		strcpy(temp,s.c_str());
		return temp;
	}
}

#endif

%inline %{
	Date DateFromSerialNumber(int serialNumber) {
		return Date(serialNumber);
	}
%}


#endif
