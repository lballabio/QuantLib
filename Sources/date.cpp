
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

/*! \file date.cpp
	\brief date- and time-related classes, typedefs and enumerations 
	
	$Source$
	$Name$
	$Log$
	Revision 1.10  2001/01/04 20:09:31  nando
	few changes: enumerations, tab/spaces, more checks, python test, bug fixed

	Revision 1.9  2000/12/27 14:05:57  lballabio
	Turned Require and Ensure functions into QL_REQUIRE and QL_ENSURE macros
	
	Revision 1.8  2000/12/14 12:32:31  lballabio
	Added CVS tags in Doxygen file documentation blocks
*/

#include "date.h"

namespace QuantLib {

    // valid date interval definition
    const int Date::minimumSerialNumber = 367;      // January 1st, 1901 included
    const int Date::maximumSerialNumber = 73050;    // December 31st, 2099 included
    const Date Date::MinimumDate = Date(Date::minimumSerialNumber);
    const Date Date::MaximumDate = Date(Date::maximumSerialNumber);
    
    const Day Date::monthOffsetStorage[]     = {
                                     0,  31,  59,  90, 120, 151, // Jan - Jun
                                   181, 212, 243, 273, 304, 334, // Jun - Dec
                                   365 // used in dayOfMonth to bracket day
                                   };

    const Day Date::monthLeapOffsetStorage[] = {
                                     0,  31,  60,  91, 121, 152, // Jan - Jun
                                   182, 213, 244, 274, 305, 335, // Jun - Dec
                                   366 // used in dayOfMonth to bracket day
                                   };

    const Day Date::monthLengthStorage[]     = {
                                   31, 28, 31, 30, 31, 30,
                                   31, 31, 30, 31, 30, 31 };

    const Day Date::monthLeapLengthStorage[] = {
                                   31, 29, 31, 30, 31, 30,
                                   31, 31, 30, 31, 30, 31 };
    
    const Day QL_PTR_CONST Date::monthOffset      = Date::monthOffsetStorage-1;
    const Day QL_PTR_CONST Date::monthLeapOffset  = Date::monthLeapOffsetStorage-1;
    const Day QL_PTR_CONST Date::monthLength      = Date::monthLengthStorage-1;
    const Day QL_PTR_CONST Date::monthLeapLength  = Date::monthLeapLengthStorage-1;


    // yearOffset and yearIsLeap enforce the 1900-2100 limit
    // easterMonday enforces too

    // the list of all December 31st in the preceding year
    // e.g. for 1901 yearOffset[1] is 366 that is December 31 1900
    const Day yearOffset[] = {
        // 1900-1909
            0,   366,   731,  1096,  1461,  1827,  2192,  2557,  2922,  3288,
        // 1910-1919
         3653,  4018,  4383,  4749,  5114,  5479,  5844,  6210,  6575,  6940,
        // 1920-1929
         7305,  7671,  8036,  8401,  8766,  9132,  9497,  9862, 10227, 10593,
        // 1930-1939
        10958, 11323, 11688, 12054, 12419, 12784, 13149, 13515, 13880, 14245,
        // 1940-1949
        14610, 14976, 15341, 15706, 16071, 16437, 16802, 17167, 17532, 17898,
        // 1950-1959
        18263, 18628, 18993, 19359, 19724, 20089, 20454, 20820, 21185, 21550,
        // 1960-1969
        21915, 22281, 22646, 23011, 23376, 23742, 24107, 24472, 24837, 25203,
        // 1970-1979
        25568, 25933, 26298, 26664, 27029, 27394, 27759, 28125, 28490, 28855,
        // 1980-1989
        29220, 29586, 29951, 30316, 30681, 31047, 31412, 31777, 32142, 32508,
        // 1990-1999
        32873, 33238, 33603, 33969, 34334, 34699, 35064, 35430, 35795, 36160,
        // 2000-2009
        36525, 36891, 37256, 37621, 37986, 38352, 38717, 39082, 39447, 39813,
        // 2010-2019
        40178, 40543, 40908, 41274, 41639, 42004, 42369, 42735, 43100, 43465,
        // 2020-2029
        43830, 44196, 44561, 44926, 45291, 45657, 46022, 46387, 46752, 47118,
        // 2030-2039
        47483, 47848, 48213, 48579, 48944, 49309, 49674, 50040, 50405, 50770,
        // 2040-2049
        51135, 51501, 51866, 52231, 52596, 52962, 53327, 53692, 54057, 54423,
        // 2050-2059
        54788, 55153, 55518, 55884, 56249, 56614, 56979, 57345, 57710, 58075,
        // 2060-2069
        58440, 58806, 59171, 59536, 59901, 60267, 60632, 60997, 61362, 61728,
        // 2070-2079
        62093, 62458, 62823, 63189, 63554, 63919, 64284, 64650, 65015, 65380,
        // 2080-2089
        65745, 66111, 66476, 66841, 67206, 67572, 67937, 68302, 68667, 69033,
        // 2090-2099
        69398, 69763, 70128, 70494, 70859, 71224, 71589, 71955, 72320, 72685,
        // 2100
        73050 };                                                                    

    static const bool yearIsLeap[] = {
        // 1900 is leap to accord with Excel's bug
        // 1900 is out of valid date range anyway
        // 1900-1909
         true,false,false,false, true,false,false,false, true,false,
        // 1910-1919
        false,false, true,false,false,false, true,false,false,false,
        // 1920-1929
         true,false,false,false, true,false,false,false, true,false,
        // 1930-1939
        false,false, true,false,false,false, true,false,false,false,
        // 1940-1949
         true,false,false,false, true,false,false,false, true,false,
        // 1950-1959
        false,false, true,false,false,false, true,false,false,false,   
        // 1960-1969
         true,false,false,false, true,false,false,false, true,false,   
        // 1970-1979
        false,false, true,false,false,false, true,false,false,false,   
        // 1980-1989
         true,false,false,false, true,false,false,false, true,false,   
        // 1990-1999
        false,false, true,false,false,false, true,false,false,false,   
        // 2000-2009
         true,false,false,false, true,false,false,false, true,false,   
        // 2010-2019
        false,false, true,false,false,false, true,false,false,false,   
        // 2020-2029
         true,false,false,false, true,false,false,false, true,false,   
        // 2030-2039
        false,false, true,false,false,false, true,false,false,false,   
        // 2040-2049
         true,false,false,false, true,false,false,false, true,false,   
        // 2050-2059
        false,false, true,false,false,false, true,false,false,false,   
        // 2060-2069
         true,false,false,false, true,false,false,false, true,false,   
        // 2070-2079
        false,false, true,false,false,false, true,false,false,false,   
        // 2080-2089
         true,false,false,false, true,false,false,false, true,false,   
        // 2090-2099
        false,false, true,false,false,false, true,false,false,false,   
        // 2100
        false };


    // constructors
    Date::Date()
    : serialNumber_(0) {}
    
    Date::Date(int serialNumber)
    : serialNumber_(serialNumber) {
        #ifdef QL_DEBUG
            QL_REQUIRE(*this >= minDate() && *this <= maxDate(),
                        "Date outside allowed range");
        #endif
    }
    
    Date::Date(Day d, Month m, Year y) {
        QL_REQUIRE(int(y) > 1900 && int(y) < 2100,
            "Date outside allowed range 1901-2099");

        QL_REQUIRE(int(m) > 0 && int(m) < 13,
            "Month outside January-December range");

        int len, offset;
        if (isLeap(y)) {
            len = monthLeapLength[m];
            offset = monthLeapOffset[m];
        } else {
            len = monthLength[m];
            offset = monthOffset[m];
        }
        QL_REQUIRE(int(d) <= len && int(d) > 0,
            "day greater than last day in month");
        
        serialNumber_ = d + offset + yearOffset[y-1900];
        #ifdef QL_DEBUG
            QL_REQUIRE(*this >= minDate() && *this <= maxDate(),
                "Date outside allowed range 1901-2099");
        #endif
    }
    
    // 1 based, Sunday = 1
    Weekday Date::weekday() const {
        int w = serialNumber_ % 7;
        return Weekday(w == 0 ? 7 : w);
    }
    
    // 1 based, February 1st = 1
    Day Date::dayOfMonth() const {
        return dayOfYear() -
          (isLeap(year()) ? monthLeapOffset[month()] : monthOffset[month()]);
    }
    
    // 1 based, January 1st = 1
    Day Date::dayOfYear() const {
        return serialNumber_ - yearOffset[year()-1900];
    }
    
    Month Date::month() const {
        Day d = dayOfYear(); // dayOfYear is 1 based
        int m = d/30 + 1;
        if (isLeap(year())) {
            while (d <= monthLeapOffset[m])
                m--;
            while (d > monthLeapOffset[m+1])
                m++;
        } else {
            while (d <= monthOffset[m])
                m--;
            while (d > monthOffset[m+1])
                m++;
        }
        return Month(m);
    }
    
    Year Date::year() const {
        Year y = serialNumber_ / 365;
        // yearOffset[y] is the December 31st of the preceding year
        if (serialNumber_ <= yearOffset[y])
            y--;
        return y+1900;
    }
    
    int Date::serialNumber() const {
        return serialNumber_;
    }
    

    // operators
    Date& Date::operator+=(int days) {
        serialNumber_ += days;
        return *this;
    }
    
    Date& Date::operator-=(int days) {
        serialNumber_ -= days;
        return *this;
    }
    
    Date& Date::operator++() {
        serialNumber_++;
        return *this;
    }
    
    Date Date::operator++(int) {
        Date temp = *this;
        serialNumber_++;
        return temp;
    }
    
    Date& Date::operator--() {
        serialNumber_--;
        return *this;
    }
    
    Date Date::operator--(int) {
        Date temp = *this;
        serialNumber_--;
        return temp;
    }
    
    Date Date::operator+(int days) const {
        return Date(serialNumber_+days);
    }
    
    Date Date::operator-(int days) const {
        return Date(serialNumber_-days);
    }
    
    Date Date::plusDays(int days) const {
        return Date(serialNumber_+days);
    }
    
    Date Date::plusWeeks(int weeks) const {
        return Date(serialNumber_+weeks*7);
    }
    
    Date Date::plusMonths(int months) const {
        Day d = dayOfMonth();
        int m = int(month())+months;
        Year y = year();
        while (m > 12) {
            m -= 12;
            y += 1;
        }
        while (m < 1) {
            m += 12;
            y -= 1;
        }
    
        QL_ENSURE(y >= 1900 && y <= 2099, "Date::plusMonths() : "
                "result must be between Jan. 1st, 1901 and Dec. 31, 2099");
    
        int length = (isLeap(y) ? monthLeapLength[m] : monthLength[m]);
        if (d > length)
            d = length;
        return Date(d,Month(m),y);
    }
    
    Date Date::plusYears(int years) const {
        Day d = dayOfMonth();
        Month m = month();
        Year y = year()+years;
    
        QL_ENSURE(y >= 1900 && y <= 2099, "Date::plusYears() : "
                "result must be between Jan. 1st, 1901 and Dec. 31, 2099");
    
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
        return yearIsLeap[y-1900];
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
