
/*
 Copyright (C) 2004 Ferdinando Ametrano
 Copyright (C) 2000-2004 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/date.hpp>
#include <ql/basicdataformatters.hpp>

namespace QuantLib {

    bool operator<(const Period& p1, const Period& p2) {
        if (p1.units() == p2.units())
            return (p1.length() < p2.length());
        if (p1.units() == Days) {
            if (p2.units() == Weeks)
                return (p1.length() < p2.length() * 7);
            else if (p2.units() == Years)
                return (p1.length() < p2.length() * 365);
            else if (p2.units() == Months)
                QL_FAIL("undecidable comparison between days and months");
            else
                QL_FAIL("unknown units");
        } else if (p1.units() == Weeks) {
            if (p2.units() == Days)
                return (p1.length() * 7 < p2.length());
            else if (p2.units() == Months || p2.units() == Years)
                QL_FAIL("undecidable comparison between "
                        "weeks and months/years");
            else
                QL_FAIL("unknown units");
        } else if (p1.units() == Months) {
            if (p2.units() == Years)
                return (p1.length() < p2.length() * 12);
            else if (p2.units() == Days || p2.units() == Weeks)
                QL_FAIL("undecidable comparison between "
                            "months and days/weeks");
            else
                QL_FAIL("unknown units");
        } else if (p1.units() == Years) {
            if (p2.units() == Days)
                return (p1.length() * 365 < p2.length());
            else if (p2.units() == Months)
                return (p1.length() * 12 < p2.length());
            else if (p2.units() == Weeks)
                QL_FAIL("undecidable outcome comparing years and weeks");
            else
                QL_FAIL("unknown units");
        } else {
            QL_FAIL("unknown units");
        }
    }


    // constructors
    Date::Date()
    : serialNumber_(BigInteger(0)) {}

    Date::Date(BigInteger serialNumber)
    : serialNumber_(serialNumber) {
        QL_REQUIRE(serialNumber >= minimumSerialNumber() &&
                   serialNumber <= maximumSerialNumber(),
                   "Date " + IntegerFormatter::toString(serialNumber) +
                   " outside allowed range [" +
                   DateFormatter::toString(minDate()) + "-" +
                   DateFormatter::toString(maxDate()) + "]");
    }

    Date::Date(Day d, Month m, Year y) {
        QL_REQUIRE(y > 1900 && y < 2100,
                   "year " + IntegerFormatter::toString(y) +
                   " out of bound. It must be in [1901,2099]");
        QL_REQUIRE(Integer(m) > 0 && Integer(m) < 13,
                   "month " + IntegerFormatter::toString(Integer(m)) +
                   " outside January-December range [1,12]");

        bool leap = isLeap(y);
        Day len = monthLength(m,leap), offset = monthOffset(m,leap);
        QL_REQUIRE(d <= len && d > 0,
                   "day outside month (" +
                   IntegerFormatter::toString(Integer(m)) + ") day-range "
                   "[1," + IntegerFormatter::toString(len) + "]");

        serialNumber_ = d + offset + yearOffset(y);
    }

    Month Date::month() const {
        Day d = dayOfYear(); // dayOfYear is 1 based
        Integer m = d/30 + 1;
        bool leap = isLeap(year());
        while (d <= monthOffset(Month(m),leap))
            m--;
        while (d > monthOffset(Month(m+1),leap))
            m++;
        return Month(m);
    }

    Year Date::year() const {
        Year y = (serialNumber_ / 365)+1900;
        // yearOffset(y) is December 31st of the preceding year
        if (serialNumber_ <= yearOffset(y))
            y--;
        return y;
    }

    Date& Date::operator+=(BigInteger days) {
        BigInteger serial = serialNumber_ + days;
        QL_REQUIRE(serial >= minimumSerialNumber() &&
                   serial <= maximumSerialNumber(),
                   "Date " + IntegerFormatter::toString(serial) +
                   "outside allowed range [" +
                   DateFormatter::toString(minDate()) + "-" +
                   DateFormatter::toString(maxDate()) + "]");
        serialNumber_ = serial;
        return *this;
    }

    Date& Date::operator+=(const Period& p) {
        serialNumber_ = advance(*this,p.length(),p.units()).serialNumber();
        return *this;
    }

    Date& Date::operator-=(BigInteger days) {
        BigInteger serial = serialNumber_ - days;
        QL_REQUIRE(serial >= minimumSerialNumber() &&
                   serial <= maximumSerialNumber(),
                   "Date " + IntegerFormatter::toString(serial) +
                   "outside allowed range [" +
                   DateFormatter::toString(minDate()) + "-" +
                   DateFormatter::toString(maxDate()) + "]");
        serialNumber_ = serial;
        return *this;
    }

    Date& Date::operator-=(const Period& p) {
        serialNumber_ = advance(*this,-p.length(),p.units()).serialNumber();
        return *this;
    }

    Date& Date::operator++() {
        BigInteger serial = serialNumber_ + 1;
        QL_REQUIRE(serial >= minimumSerialNumber() &&
                   serial <= maximumSerialNumber(),
                   "Date " + IntegerFormatter::toString(serial) +
                   "outside allowed range [" +
                   DateFormatter::toString(minDate()) + "-" +
                   DateFormatter::toString(maxDate()) + "]");
        serialNumber_ = serial;
        return *this;
    }

    Date Date::operator++(int ) {
        Date temp = *this;
        BigInteger serial = serialNumber_ + 1;
        QL_REQUIRE(serial >= minimumSerialNumber() &&
                   serial <= maximumSerialNumber(),
                   "Date " + IntegerFormatter::toString(serial) +
                   "outside allowed range [" +
                   DateFormatter::toString(minDate()) + "-" +
                   DateFormatter::toString(maxDate()) + "]");
        serialNumber_ = serial;
        return temp;
    }

    Date& Date::operator--() {
        BigInteger serial = serialNumber_ - 1;
        QL_REQUIRE(serial >= minimumSerialNumber() &&
                   serial <= maximumSerialNumber(),
                   "Date " + IntegerFormatter::toString(serial) +
                   "outside allowed range [" +
                   DateFormatter::toString(minDate()) + "-" +
                   DateFormatter::toString(maxDate()) + "]");
        serialNumber_ = serial;
        return *this;
    }

    Date Date::operator--(int ) {
        Date temp = *this;
        BigInteger serial = serialNumber_ - 1;
        QL_REQUIRE(serial >= minimumSerialNumber() &&
                   serial <= maximumSerialNumber(),
                   "Date " + IntegerFormatter::toString(serial) +
                   "outside allowed range [" +
                   DateFormatter::toString(minDate()) + "-" +
                   DateFormatter::toString(maxDate()) + "]");
        serialNumber_ = serial;
        return temp;
    }

    Date Date::todaysDate() {
        QL_TIME_T t;

        if (QL_TIME(&t) == QL_TIME_T(-1)) // -1 means time() didn't work
            return Date();
        QL_TM *gt = QL_GMTIME(&t);
        return Date(Day(gt->tm_mday),
                    Month(gt->tm_mon+1),
                    Year(gt->tm_year+1900));
    }

    Date Date::minDate() {
        static const Date minimumDate(minimumSerialNumber());
        return minimumDate;
    }

    Date Date::maxDate() {
        static const Date maximumDate(maximumSerialNumber());
        return maximumDate;
    }

    Date Date::advance(const Date& date, Integer n, TimeUnit units) {
        switch (units) {
          case Days:
            return date + n;
          case Weeks:
            return date + 7*n;
            break;
          case Months: {
            Day d = date.dayOfMonth();
            Integer m = Integer(date.month())+n;
            Year y = date.year();
            while (m > 12) {
                m -= 12;
                y += 1;
            }
            while (m < 1) {
                m += 12;
                y -= 1;
            }

            QL_ENSURE(y >= 1900 && y <= 2099,
                      "year " + IntegerFormatter::toString(y) +
                      " out of bound. It must be in [1901,2099]");

            Integer length = monthLength(Month(m), isLeap(y));
            if (d > length)
                d = length;

            return Date(d, Month(m), y);
          }
          case Years: {
              Day d = date.dayOfMonth();
              Month m = date.month();
              Year y = date.year()+n;

              QL_ENSURE(y >= 1900 && y <= 2099,
                        "year " + IntegerFormatter::toString(y) +
                        " out of bound. It must be in [1901,2099]");

              if (d == 29 && m == February && !isLeap(y))
                  d = 28;

              return Date(d,m,y);
          }
          default:
            QL_FAIL("undefined time units");
        }
    }

    bool Date::isLeap(Year y) {
        static const bool YearIsLeap[] = {
            // 1900 is leap in agreement with Excel's bug
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
            false
        };
        return YearIsLeap[y-1900];
    }

    Date Date::nextIMMdate(const Date& date) {
        Year y = date.year();
        Month m = date.month();

        Size skipMonths = 3-(m%3);
        if (skipMonths != 3 || date.dayOfMonth() > 21) {
            skipMonths += Size(m);
            if (skipMonths<=12) {
                m = Month(skipMonths);
            } else {
                m = Month(skipMonths-12);
                y += 1;
            }
        // date is in a IMM month and in the IMM week [15,21]
        } else if (date.dayOfMonth() > 14) {
            Date nextWednesday = nextWeekday(date, Wednesday);
            if (nextWednesday.dayOfMonth() <= 21)
                return nextWednesday;
            else {
                if (m <= 9) {
                    m = Month(Size(m)+3);
                } else {
                    m = Month(Size(m)-9);
                    y += 1;
                }
            }
        }

        return nthWeekday(3, Wednesday, m, y);
    }

    Date Date::nextWeekday(const Date& d, Weekday dayOfWeek) {
        Weekday wd = d.weekday();
        return d + ((wd>dayOfWeek ? 7 : 0) - wd + dayOfWeek);
    }

    Date Date::nthWeekday(Size nth, Weekday dayOfWeek,
                          Month m, Year y) {
        QL_REQUIRE(nth>0,
                   "zeroth day of week in a given (month, year) is undefined");
        Weekday first = Date(1, m, y).weekday();
        Size skip = nth - (dayOfWeek>=first ? 1 : 0);
        return Date(1 + dayOfWeek-first + skip*7, m, y);
    }

    Integer Date::monthLength(Month m, bool leapYear) {
        static const Integer MonthLength[] = {
            31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
        };
        static const Integer MonthLeapLength[] = {
            31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
        };
        return (leapYear? MonthLeapLength[m-1] : MonthLength[m-1]);
    }

    Integer Date::monthOffset(Month m, bool leapYear) {
        static const Integer MonthOffset[] = {
              0,  31,  59,  90, 120, 151,   // Jan - Jun
            181, 212, 243, 273, 304, 334,   // Jun - Dec
            365     // used in dayOfMonth to bracket day
        };
        static const Integer MonthLeapOffset[] = {
              0,  31,  60,  91, 121, 152,   // Jan - Jun
            182, 213, 244, 274, 305, 335,   // Jun - Dec
            366     // used in dayOfMonth to bracket day
        };
        return (leapYear? MonthLeapOffset[m-1] : MonthOffset[m-1]);
    }

    BigInteger Date::yearOffset(Year y) {
        // the list of all December 31st in the preceding year
        // e.g. for 1901 yearOffset[1] is 366, that is, December 31 1900
        static const BigInteger YearOffset[] = {
            // 1900-1909
                0,  366,  731, 1096, 1461, 1827, 2192, 2557, 2922, 3288,
            // 1910-1919
             3653, 4018, 4383, 4749, 5114, 5479, 5844, 6210, 6575, 6940,
            // 1920-1929
             7305, 7671, 8036, 8401, 8766, 9132, 9497, 9862,10227,10593,
            // 1930-1939
            10958,11323,11688,12054,12419,12784,13149,13515,13880,14245,
            // 1940-1949
            14610,14976,15341,15706,16071,16437,16802,17167,17532,17898,
            // 1950-1959
            18263,18628,18993,19359,19724,20089,20454,20820,21185,21550,
            // 1960-1969
            21915,22281,22646,23011,23376,23742,24107,24472,24837,25203,
            // 1970-1979
            25568,25933,26298,26664,27029,27394,27759,28125,28490,28855,
            // 1980-1989
            29220,29586,29951,30316,30681,31047,31412,31777,32142,32508,
            // 1990-1999
            32873,33238,33603,33969,34334,34699,35064,35430,35795,36160,
            // 2000-2009
            36525,36891,37256,37621,37986,38352,38717,39082,39447,39813,
            // 2010-2019
            40178,40543,40908,41274,41639,42004,42369,42735,43100,43465,
            // 2020-2029
            43830,44196,44561,44926,45291,45657,46022,46387,46752,47118,
            // 2030-2039
            47483,47848,48213,48579,48944,49309,49674,50040,50405,50770,
            // 2040-2049
            51135,51501,51866,52231,52596,52962,53327,53692,54057,54423,
            // 2050-2059
            54788,55153,55518,55884,56249,56614,56979,57345,57710,58075,
            // 2060-2069
            58440,58806,59171,59536,59901,60267,60632,60997,61362,61728,
            // 2070-2079
            62093,62458,62823,63189,63554,63919,64284,64650,65015,65380,
            // 2080-2089
            65745,66111,66476,66841,67206,67572,67937,68302,68667,69033,
            // 2090-2099
            69398,69763,70128,70494,70859,71224,71589,71955,72320,72685,
            // 2100
            73050
        };
        return YearOffset[y-1900];
    }

    BigInteger Date::minimumSerialNumber() {
        return 367;       // Jan 1st, 1901
    }

    BigInteger Date::maximumSerialNumber() {
        return 73050;    // Dec 31st, 2099
    }


    std::string DateFormatter::toString(const Date& d,
                                        DateFormatter::Format f) {
        static const std::string monthName[] = {
            "January", "February", "March", "April", "May", "June",
            "July", "August", "September", "October", "November", "December"
        };
        std::string output;
        if (d == Date()) {
            output = "Null date";
        } else {
            Integer dd = d.dayOfMonth(), mm = Integer(d.month()),
                    yyyy = d.year();
            switch (f) {
              case Long:
                output = monthName[mm-1] + " ";
                output += IntegerFormatter::toString(dd);
                switch (dd) {
                  case 1:
                  case 21:
                  case 31:
                    output += "st, ";
                    break;
                  case 2:
                  case 22:
                    output += "nd, ";
                    break;
                  case 3:
                  case 23:
                    output += "rd, ";
                    break;
                  default:
                    output += "th, ";
                }
                output += IntegerFormatter::toString(yyyy);
                break;
              case Short:
                output = (mm < 10 ? "0" : "") +
                         IntegerFormatter::toString(mm);
                output += (dd < 10 ? "/0" : "/") +
                         IntegerFormatter::toString(dd);
                output += "/" + IntegerFormatter::toString(yyyy);
                break;
              case ISO:
                output = IntegerFormatter::toString(yyyy);
                output += (mm < 10 ? "-0" : "-") +
                         IntegerFormatter::toString(mm);
                output += (dd < 10 ? "-0" : "-") +
                         IntegerFormatter::toString(dd);
                break;
              default:
                QL_FAIL("unknown date format");
            }
        }
        return output;
    }


    std::string WeekdayFormatter::toString(Weekday wd,
                                           WeekdayFormatter::Format f) {
        static const std::string weekDaysLongNames[] = {
            "Sunday", "Monday", "Tuesday", "Wednesday",
            "Thursday", "Friday", "Saturday"
        };
        static const std::string weekDaysShortNames[] = {
            "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
        };
        static const std::string weekDaysVeryShortNames[] = {
            "Su", "Mo", "Tu", "We", "Th", "Fr", "Sa"
        };
        std::string output;
        switch (f) {
          case Long:
            output = weekDaysLongNames[wd-1];
            break;
          case Short:
            output = weekDaysShortNames[wd-1];
            break;
          case Shortest:
            output = weekDaysVeryShortNames[wd-1];
            break;
          default:
            QL_FAIL("unknown weekday format");
        }
        return output;
    }

    std::string FrequencyFormatter::toString(Frequency freq) {

        switch (freq) {
            case NoFrequency:
            return std::string("no frequency");
            break;
            case Once:
            return std::string("once");
            break;
            case Annual:
            return std::string("annual");
            break;
            case Semiannual:
            return std::string("semiannual");
            break;
            case EveryFourthMonth:
            return std::string("every-fourth-month");
            break;
            case Quarterly:
            return std::string("quarterly");
            break;
            case Bimonthly:
            return std::string("bimonthly");
            break;
            case Monthly:
            return std::string("monthly");
            break;
            default:
                QL_FAIL("unknown frequency ("+
                    IntegerFormatter::toString(freq)+")");
        }
    }

}
