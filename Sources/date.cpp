
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

// $Source$

// $Log$
// Revision 1.7  2000/12/13 18:10:41  nando
// CVS keyword added
//

#include "date.h"

namespace QuantLib {

	int MinimumSerialNumber = 367;		// January 1st, 1901
	Date MinimumDate = Date(MinimumSerialNumber);
	int maximumSerialNumber = 73050;	// December 31st, 2099
	Date MaximumDate = Date(maximumSerialNumber);
	
	const Day monthOffset[] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 };
	const Day monthLength[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	
	const Day monthLeapOffset[] = { 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366 };
	const Day monthLeapLength[] = { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	
	const Day yearOffset[] = {
			1,   367,   732,  1097,  1462,  1828,  2193,  2558,  2923,  3289,	// 1900-1909
		 3654,  4019,  4384,  4750,  5115,  5480,  5845,  6211,  6576,  6941,	// 1910-1919
		 7306,  7672,  8037,  8402,  8767,  9133,  9498,  9863, 10228, 10594,	// 1920-1929
		10959, 11324, 11689, 12055, 12420, 12785, 13150, 13516, 13881, 14246,	// 1930-1939
		14611, 14977, 15342, 15707, 16072, 16438, 16803, 17168, 17533, 17899,	// 1940-1949
		18264, 18629, 18994, 19360, 19725, 20090, 20455, 20821, 21186, 21551,	// 1950-1959
		21916, 22282, 22647, 23012, 23377, 23743, 24108, 24473, 24838, 25204,	// 1960-1969
		25569, 25934, 26299, 26665, 27030, 27395, 27760, 28126, 28491, 28856,	// 1970-1979
		29221, 29587, 29952, 30317, 30682, 31048, 31413, 31778, 32143, 32509,	// 1980-1989
		32874, 33239, 33604, 33970, 34335, 34700, 35065, 35431, 35796, 36161,	// 1990-1999
		36526, 36892, 37257, 37622, 37987, 38353, 38718, 39083, 39448, 39814,	// 2000-2009
		40179, 40544, 40909, 41275, 41640, 42005, 42370, 42736, 43101, 43466,	// 2010-2019
		43831, 44197, 44562, 44927, 45292, 45658, 46023, 46388, 46753, 47119,	// 2020-2029
		47484, 47849, 48214, 48580, 48945, 49310, 49675, 50041, 50406, 50771,	// 2030-2039
		51136, 51502, 51867, 52232, 52597, 52963, 53328, 53693, 54058, 54424,	// 2040-2049
		54789, 55154, 55519, 55885, 56250, 56615, 56980, 57346, 57711, 58076,	// 2050-2059
		58441, 58807, 59172, 59537, 59902, 60268, 60633, 60998, 61363, 61729,	// 2060-2069
		62094, 62459, 62824, 63190, 63555, 63920, 64285, 64651, 65016, 65381,	// 2070-2079
		65746, 66112, 66477, 66842, 67207, 67573, 67938, 68303, 68668, 69034,	// 2080-2089
		69399, 69764, 70129, 70495, 70860, 71225, 71590, 71956, 72321, 72686,	// 2090-2099
		73051 };																// 2100
	
	Date::Date()
	: theSerialNumber(0) {}
	
	Date::Date(int serialNumber)
	: theSerialNumber(serialNumber) {
		#ifdef QL_DEBUG
			Require(*this >= minDate() && *this <= maxDate(), "Date outside allowed range");
		#endif
	}
	
	Date::Date(Day d, Month m, Year y) {
		Require(y >= 1900 && y <= 2100, "Date outside allowed range");
		theSerialNumber = yearOffset[y-1900]+(isLeap(y) ? monthLeapOffset[m] : monthOffset[m])+d-1;
		#ifdef QL_DEBUG
			Require(*this >= minDate() && *this <= maxDate(), "Date outside allowed range");
		#endif
	}
	
	Date& Date::operator+=(int days) {
		theSerialNumber += days;
		return *this;
	}
	
	Date& Date::operator-=(int days) {
		theSerialNumber -= days;
		return *this;
	}
	
	Weekday Date::dayOfWeek() const {
		return Weekday(theSerialNumber%7);
	}
	
	Day Date::dayOfMonth() const {
		return dayOfYear()-(isLeap(year()) ? monthLeapOffset[month()] : monthOffset[month()])+1;
	}
	
	Day Date::dayOfYear() const {
		return theSerialNumber-yearOffset[year()-1900];
	}
	
	Month Date::month() const {
		bool leap = isLeap(year());
		Day d = dayOfYear();
		int m = d/30;
		while (d < (leap ? monthLeapOffset[m] : monthOffset[m]))
			m--;
		while (d >= (leap ? monthLeapOffset[m+1] : monthOffset[m+1]))
			m++;
		return Month(m);
	}
	
	Year Date::year() const {
		Year y = theSerialNumber/365;
		while (theSerialNumber < yearOffset[y])
			y--;
		return y+1900;
	}
	
	int Date::serialNumber() const {
		return theSerialNumber;
	}
	
	Date& Date::operator++() {
		theSerialNumber++;
		return *this;
	}
	
	Date Date::operator++(int) {
		Date temp = *this;
		theSerialNumber++;
		return temp;
	}
	
	Date& Date::operator--() {
		theSerialNumber--;
		return *this;
	}
	
	Date Date::operator--(int) {
		Date temp = *this;
		theSerialNumber--;
		return temp;
	}
	
	Date Date::operator+(int days) const {
		return Date(theSerialNumber+days);
	}
	
	Date Date::operator-(int days) const {
		return Date(theSerialNumber-days);
	}
	
	Date Date::plusDays(int days) const {
		return Date(theSerialNumber+days);
	}
	
	Date Date::plusWeeks(int weeks) const {
		return Date(theSerialNumber+weeks*7);
	}
	
	Date Date::plusMonths(int months) const {
		Day d = dayOfMonth();
		int m = int(month())+months;
		Year y = year();
		while (m >= 12) {
			m -= 12;
			y += 1;
		}
		while (m < 0) {
			m += 12;
			y -= 1;
		}
	
		Ensure(y >= 1900 && y <= 2099, "Date::plusMonths() : result must be between Jan. 1st, 1901 and Dec. 31, 2099");
	
		int length = (isLeap(y) ? monthLeapLength[m] : monthLength[m]);
		if (d > length)
			d = length;
		return Date(d,Month(m),y);
	}
	
	Date Date::plusYears(int years) const {
		Day d = dayOfMonth();
		Month m = month();
		Year y = year()+years;
	
		Ensure(y >= 1900 && y <= 2099, "Date::plusYears() : result must be between Jan. 1st, 1901 and Dec. 31, 2099");
	
		if (d == 29 && m == February && !isLeap(y))
			d = 28;
		return Date(d,m,y);
	}
	
	Date Date::plus(int units, TimeUnit theUnit) const {
		Date d;
		switch (theUnit) {
		case Days:
			d = plusDays(units);
			break;
		case Weeks:
			d = plusWeeks(units);
			break;
		case Months:
			d = plusMonths(units);
			break;
		case Years:
			d = plusYears(units);
			break;
		default:
			throw IllegalArgumentError("Date::plus : undefined time units");
		}
		return d;
	}
	
	Date Date::minDate() {
		return MinimumDate;
	}
	
	Date Date::maxDate() {
		return MaximumDate;
	}
	
	bool Date::isLeap(Year y) {
		static const bool YearIsLeap[] = {
			 true, false, false, false,  true, false, false, false,  true, false,	// 1900-1909
			false, false,  true, false, false, false,  true, false, false, false,	// 1910-1919
			 true, false, false, false,  true, false, false, false,  true, false,	// 1920-1929
			false, false,  true, false, false, false,  true, false, false, false,	// 1930-1939
			 true, false, false, false,  true, false, false, false,  true, false,	// 1940-1949
			false, false,  true, false, false, false,  true, false, false, false,	// 1950-1959
			 true, false, false, false,  true, false, false, false,  true, false,	// 1960-1969
			false, false,  true, false, false, false,  true, false, false, false,	// 1970-1979
			 true, false, false, false,  true, false, false, false,  true, false,	// 1980-1989
			false, false,  true, false, false, false,  true, false, false, false,	// 1990-1999
			 true, false, false, false,  true, false, false, false,  true, false,	// 2000-2009
			false, false,  true, false, false, false,  true, false, false, false,	// 2010-2019
			 true, false, false, false,  true, false, false, false,  true, false,	// 2020-2029
			false, false,  true, false, false, false,  true, false, false, false,	// 2030-2039
			 true, false, false, false,  true, false, false, false,  true, false,	// 2040-2049
			false, false,  true, false, false, false,  true, false, false, false,	// 2050-2059
			 true, false, false, false,  true, false, false, false,  true, false,	// 2060-2069
			false, false,  true, false, false, false,  true, false, false, false,	// 2070-2079
			 true, false, false, false,  true, false, false, false,  true, false,	// 2080-2089
			false, false,  true, false, false, false,  true, false, false, false,	// 2090-2099
			 true };																// 2100
		return YearIsLeap[y-1900];
	}
	
	int operator-(const Date& d1, const Date& d2) {
		return d1.serialNumber()-d2.serialNumber();
	}
	
	bool operator==(const Date& d1, const Date& d2) {
		return (d1.serialNumber() == d2.serialNumber());
	}
	
	bool operator!=(const Date& d1, const Date& d2) {
		return (d1.serialNumber() != d2.serialNumber());
	}
	
	bool operator<(const Date& d1, const Date& d2) {
		return (d1.serialNumber() < d2.serialNumber());
	}
	
	bool operator<=(const Date& d1, const Date& d2) {
		return (d1.serialNumber() <= d2.serialNumber());
	}
	
	bool operator>(const Date& d1, const Date& d2) {
		return (d1.serialNumber() > d2.serialNumber());
	}
	
	bool operator>=(const Date& d1, const Date& d2) {
		return (d1.serialNumber() >= d2.serialNumber());
	}

}
