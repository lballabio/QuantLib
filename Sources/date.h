
/* 
Copyright (C) 2000 Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

#ifndef quantlib_date_h
#define quantlib_date_h

#include "qldefines.h"
#include "qlerrors.h"

QL_BEGIN_NAMESPACE(QuantLib)

typedef double Time;

typedef int Day;
enum Weekday { Saturday, Sunday, Monday, Tuesday, Wednesday, Thursday, Friday };
enum Month { January, February, March, April, May, June, 
	July, August, September, October, November, December };
typedef int Year;

enum TimeUnit { Days, Weeks, Months, Years };

// Date

class Date {
  public:
	// constructors
	Date();
	explicit Date(int serialNumber); // serial number as given by Applix or Excel
	Date(Day d, Month m, Year y);
	// computed assignment
	Date& operator+=(int days);
	Date& operator-=(int days);
	// access functions
	Weekday dayOfWeek() const;
	Day dayOfMonth() const;
	Day dayOfYear() const;		// zero-based
	Month month() const;
	Year year() const;
	int serialNumber() const;
	// increment/decrement dates
	Date& operator++();
	Date operator++(int);
	Date& operator--();
	Date operator--(int);
	Date operator+(int days) const;
	Date operator-(int days) const;
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
  private:
	int theSerialNumber;
};

// difference
int operator-(const Date&, const Date&);
// comparisons
bool operator==(const Date&, const Date&);
bool operator!=(const Date&, const Date&);
bool operator<(const Date&, const Date&);
bool operator<=(const Date&, const Date&);
bool operator>(const Date&, const Date&);
bool operator>=(const Date&, const Date&);

QL_END_NAMESPACE(QuantLib)


#endif
