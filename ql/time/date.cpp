/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005, 2006, 2007 StatPro Italia srl
 Copyright (C) 2004, 2005, 2006 Ferdinando Ametrano
 Copyright (C) 2006 Katiuscia Manzoni
 Copyright (C) 2006 Toyin Akin

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/time/date.hpp>
#include <ql/utilities/dataformatters.hpp>
#include <ql/errors.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <iomanip>
#include <ctime>

#if defined(BOOST_NO_STDC_NAMESPACE)
    namespace std { using ::time; using ::time_t; using ::tm;
                    using ::gmtime; using ::localtime; }
#endif

namespace QuantLib {

    // constructors
    Date::Date()
    : serialNumber_(BigInteger(0)) {}

    Date::Date(BigInteger serialNumber)
    : serialNumber_(serialNumber) {
        checkSerialNumber(serialNumber);
    }

    Date::Date(Day d, Month m, Year y) {
        QL_REQUIRE(y > 1900 && y < 2200,
                   "year " << y << " out of bound. It must be in [1901,2199]");
        QL_REQUIRE(Integer(m) > 0 && Integer(m) < 13,
                   "month " << Integer(m)
                   << " outside January-December range [1,12]");

        bool leap = isLeap(y);
        Day len = monthLength(m,leap), offset = monthOffset(m,leap);
        QL_REQUIRE(d <= len && d > 0,
                   "day outside month (" << Integer(m) << ") day-range "
                   << "[1," << len << "]");

        serialNumber_ = d + offset + yearOffset(y);
    }

    Month Date::month() const {
        Day d = dayOfYear(); // dayOfYear is 1 based
        Integer m = d/30 + 1;
        bool leap = isLeap(year());
        while (d <= monthOffset(Month(m),leap))
            --m;
        while (d > monthOffset(Month(m+1),leap))
            ++m;
        return Month(m);
    }

    Year Date::year() const {
        Year y = (serialNumber_ / 365)+1900;
        // yearOffset(y) is December 31st of the preceding year
        if (serialNumber_ <= yearOffset(y))
            --y;
        return y;
    }

    Date& Date::operator+=(BigInteger days) {
        BigInteger serial = serialNumber_ + days;
        checkSerialNumber(serial);
        serialNumber_ = serial;
        return *this;
    }

    Date& Date::operator+=(const Period& p) {
        serialNumber_ = advance(*this,p.length(),p.units()).serialNumber();
        return *this;
    }

    Date& Date::operator-=(BigInteger days) {
        BigInteger serial = serialNumber_ - days;
        checkSerialNumber(serial);
        serialNumber_ = serial;
        return *this;
    }

    Date& Date::operator-=(const Period& p) {
        serialNumber_ = advance(*this,-p.length(),p.units()).serialNumber();
        return *this;
    }

    Date& Date::operator++() {
        BigInteger serial = serialNumber_ + 1;
        checkSerialNumber(serial);
        serialNumber_ = serial;
        return *this;
    }

    Date Date::operator++(int ) {
        Date old(*this);
        ++*this; // use the pre-increment
        return old;
    }

    Date& Date::operator--() {
        BigInteger serial = serialNumber_ - 1;
        checkSerialNumber(serial);
        serialNumber_ = serial;
        return *this;
    }

    Date Date::operator--(int ) {
        Date old(*this);
        --*this; // use the pre-decrement
        return old;
    }

    Date Date::todaysDate() {
        std::time_t t;

        if (std::time(&t) == std::time_t(-1)) // -1 means time() didn't work
            return Date();
        std::tm *lt = std::localtime(&t);
        return Date(Day(lt->tm_mday),
                    Month(lt->tm_mon+1),
                    Year(lt->tm_year+1900));
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

            QL_ENSURE(y >= 1900 && y <= 2199,
                      "year " << y << " out of bounds. "
                      << "It must be in [1901,2199]");

            Integer length = monthLength(Month(m), isLeap(y));
            if (d > length)
                d = length;

            return Date(d, Month(m), y);
          }
          case Years: {
              Day d = date.dayOfMonth();
              Month m = date.month();
              Year y = date.year()+n;

              QL_ENSURE(y >= 1900 && y <= 2199,
                        "year " << y << " out of bounds. "
                        << "It must be in [1901,2199]");

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
            // 2100-2109
            false,false,false,false, true,false,false,false, true,false,
            // 2110-2119
            false,false, true,false,false,false, true,false,false,false,
            // 2120-2129
             true,false,false,false, true,false,false,false, true,false,
            // 2130-2139
            false,false, true,false,false,false, true,false,false,false,
            // 2140-2149
             true,false,false,false, true,false,false,false, true,false,
            // 2150-2159
            false,false, true,false,false,false, true,false,false,false,
            // 2160-2169
             true,false,false,false, true,false,false,false, true,false,
            // 2170-2179
            false,false, true,false,false,false, true,false,false,false,
            // 2180-2189
             true,false,false,false, true,false,false,false, true,false,
            // 2190-2199
            false,false, true,false,false,false, true,false,false,false,
            // 2200
            false
        };
        QL_REQUIRE(y>=1900 && y<=2200, "year outside valid range");
        return YearIsLeap[y-1900];
    }


    Date Date::nextWeekday(const Date& d, Weekday dayOfWeek) {
        Weekday wd = d.weekday();
        return d + ((wd>dayOfWeek ? 7 : 0) - wd + dayOfWeek);
    }

    Date Date::nthWeekday(Size nth, Weekday dayOfWeek,
                          Month m, Year y) {
        QL_REQUIRE(nth>0,
                   "zeroth day of week in a given (month, year) is undefined");
        QL_REQUIRE(nth<6,
                   "no more than 5 weekday in a given (month, year)");
        Weekday first = Date(1, m, y).weekday();
        Size skip = nth - (dayOfWeek>=first ? 1 : 0);
        return Date((1 + dayOfWeek + skip*7) - first, m, y);
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
            // 2100-2109
            73050,73415,73780,74145,74510,74876,75241,75606,75971,76337,
            // 2110-2119
            76702,77067,77432,77798,78163,78528,78893,79259,79624,79989,
            // 2120-2129
            80354,80720,81085,81450,81815,82181,82546,82911,83276,83642,
            // 2130-2139
            84007,84372,84737,85103,85468,85833,86198,86564,86929,87294,
            // 2140-2149
            87659,88025,88390,88755,89120,89486,89851,90216,90581,90947,
            // 2150-2159
            91312,91677,92042,92408,92773,93138,93503,93869,94234,94599,
            // 2160-2169
            94964,95330,95695,96060,96425,96791,97156,97521,97886,98252,
            // 2170-2179
            98617,98982,99347,99713,100078,100443,100808,101174,101539,101904,
            // 2180-2189
            102269,102635,103000,103365,103730,104096,104461,104826,105191,105557,
            // 2190-2199
            105922,106287,106652,107018,107383,107748,108113,108479,108844,109209,
            // 2200
            109574
        };
        return YearOffset[y-1900];
    }

    BigInteger Date::minimumSerialNumber() {
        return 367;       // Jan 1st, 1901
    }

    BigInteger Date::maximumSerialNumber() {
        return 109574;    // Dec 31st, 2199
    }

    void Date::checkSerialNumber(BigInteger serialNumber) {
        QL_REQUIRE(serialNumber >= minimumSerialNumber() &&
                   serialNumber <= maximumSerialNumber(),
                   "Date's serial number (" << serialNumber << ") outside "
                   "allowed range [" << minimumSerialNumber() <<
                   "-" << maximumSerialNumber() << "], i.e. [" <<
                   minDate() << "-" << maxDate() << "]");
    }

    // month formatting

    std::ostream& operator<<(std::ostream& out, Month m) {
        switch (m) {
          case January:
            return out << "January";
          case February:
            return out << "February";
          case March:
            return out << "March";
          case April:
            return out << "April";
          case May:
            return out << "May";
          case June:
            return out << "June";
          case July:
            return out << "July";
          case August:
            return out << "August";
          case September:
            return out << "September";
          case October:
            return out << "October";
          case November:
            return out << "November";
          case December:
            return out << "December";
          default:
            QL_FAIL("unknown month (" << Integer(m) << ")");
        }
    }


    // date formatting

    std::ostream& operator<<(std::ostream& out, const Date& d) {
        return out << io::long_date(d);
    }

    namespace detail {

        std::ostream& operator<<(std::ostream& out,
                                 const short_date_holder& holder) {
            const Date& d = holder.d;
            if (d == Date()) {
                out << "null date";
            } else {
                Integer dd = d.dayOfMonth(), mm = Integer(d.month()),
                        yyyy = d.year();
                char filler = out.fill();
                out << std::setw(2) << std::setfill('0') << mm << "/";
                out << std::setw(2) << std::setfill('0') << dd << "/";
                out << yyyy;
                out.fill(filler);
            }
            return out;
        }

        std::ostream& operator<<(std::ostream& out,
                                 const long_date_holder& holder) {
            const Date& d = holder.d;
            if (d == Date()) {
                out << "null date";
            } else {
                out << d.month() << " ";
                out << io::ordinal(d.dayOfMonth()) << ", ";
                out << d.year();
            }
            return out;
        }

        std::ostream& operator<<(std::ostream& out,
                                 const iso_date_holder& holder) {
            const Date& d = holder.d;
            if (d == Date()) {
                out << "null date";
            } else {
                Integer dd = d.dayOfMonth(), mm = Integer(d.month()),
                        yyyy = d.year();
                char filler = out.fill();
                out << yyyy << "-";
                out << std::setw(2) << std::setfill('0') << mm << "-";
                out << std::setw(2) << std::setfill('0') << dd;
                out.fill(filler);
            }
            return out;
        }

        std::ostream& operator<<(std::ostream& out,
                                 const formatted_date_holder& holder) {
            using namespace boost::gregorian;
            const Date& d = holder.d;
            if (d == Date()) {
                out << "null date";
            } else {
                date boostDate(d.year(), d.month(), d.dayOfMonth());
                out.imbue(std::locale(std::locale(),
                                      new date_facet(holder.f.c_str())));
                out << boostDate;
            }
            return out;
        }

    }

    namespace io {

        detail::short_date_holder short_date(const Date& d) {
            return detail::short_date_holder(d);
        }

        detail::long_date_holder long_date(const Date& d) {
            return detail::long_date_holder(d);
        }

        detail::iso_date_holder iso_date(const Date& d) {
            return detail::iso_date_holder(d);
        }

        detail::formatted_date_holder formatted_date(const Date& d,
                                                     const std::string& f) {
            return detail::formatted_date_holder(d, f);
        }

    }

}
