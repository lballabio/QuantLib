
/*
 Copyright (C) 2004, 2005 Ferdinando Ametrano
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

/*! \file date.hpp
    \brief date- and time-related classes, typedefs and enumerations
*/

#ifndef quantlib_date_hpp
#define quantlib_date_hpp

#include <ql/errors.hpp>
#include <ql/types.hpp>
#include <utility>
#include <functional>
#include <ostream>

namespace QuantLib {

    //! Day number
    /*! \ingroup datetime */
    typedef Integer Day;

    /*! Day's serial number MOD 7;
        WEEKDAY Excel function is the same except for Sunday = 7.

        \ingroup datetime
    */
    enum Weekday { Sunday    = 1,
                   Monday    = 2,
                   Tuesday   = 3,
                   Wednesday = 4,
                   Thursday  = 5,
                   Friday    = 6,
                   Saturday  = 7,
                   Sun = 1,
                   Mon = 2,
                   Tue = 3,
                   Wed = 4,
                   Thu = 5,
                   Fri = 6,
                   Sat = 7
    };

    /*! \relates Weekday */
    std::ostream& operator<<(std::ostream&, const Weekday&);

    namespace detail {

        struct long_weekday_holder {
            long_weekday_holder(Weekday d) : d(d) {}
            Weekday d;
        };
        std::ostream& operator<<(std::ostream&, const long_weekday_holder&);

        struct short_weekday_holder {
            short_weekday_holder(Weekday d) : d(d) {}
            Weekday d;
        };
        std::ostream& operator<<(std::ostream&, const short_weekday_holder&);

        struct shortest_weekday_holder {
            shortest_weekday_holder(Weekday d) : d(d) {}
            Weekday d;
        };
        std::ostream& operator<<(std::ostream&,
                                 const shortest_weekday_holder&);

    }

    namespace io {

        //! output weekdays in long format
        /*! \ingroup manips */
        detail::long_weekday_holder long_weekday(Weekday);

        //! output weekdays in short format (three letters)
        /*! \ingroup manips */
        detail::short_weekday_holder short_weekday(Weekday);

        //! output weekdays in shortest format (two letters)
        /*! \ingroup manips */
        detail::shortest_weekday_holder shortest_weekday(Weekday);

    }


    //! Month names
    /*! \ingroup datetime */
    enum Month { January   = 1,
                 February  = 2,
                 March     = 3,
                 April     = 4,
                 May       = 5,
                 June      = 6,
                 July      = 7,
                 August    = 8,
                 September = 9,
                 October   = 10,
                 November  = 11,
                 December  = 12,
                 Jan = 1,
                 Feb = 2,
                 Mar = 3,
                 Apr = 4,
                 Jun = 6,
                 Jul = 7,
                 Aug = 8,
                 Sep = 9,
                 Oct = 10,
                 Nov = 11,
                 Dec = 12
    };

    /*! \relates Month */
    std::ostream& operator<<(std::ostream&, Month);


    //! Main cycle of the International Money Market (a.k.a. IMM) Months
    /*! \ingroup datetime */
    enum IMMMonth { H = 3,
                    M = 6,
                    U = 9,
                    Z = 12
    };

    //! Year number
    /*! \ingroup datetime */
    typedef Integer Year;

    //! Frequency of events
    /*! \ingroup datetime */
    enum Frequency { NoFrequency = -1,     //!< null frequency
                     Once = 0,             //!< only once, e.g., a zero-coupon
                     Annual = 1,           //!< once a year
                     Semiannual = 2,       //!< twice a year
                     EveryFourthMonth = 3, //!< every fourth month
                     Quarterly = 4,        //!< every third month
                     Bimonthly = 6,        //!< every second month
                     Monthly = 12          //!< once a month
    };

    /*! \relates Frequency */
    std::ostream& operator<<(std::ostream&, Frequency);

    //! Units used to describe time periods
    /*! \ingroup datetime */
    enum TimeUnit { Days,
                    Weeks,
                    Months,
                    Years
    };

    //! Time period described by a number of a given time unit
    /*! \ingroup datetime */
    class Period {
      public:
        Period()
        : length_(0), units_(Days) {}
        Period(Integer n, TimeUnit units)
        : length_(n), units_(units) {}
        Integer length() const { return length_; }
        TimeUnit units() const { return units_; }
      private:
        Integer length_;
        TimeUnit units_;
    };

    /*! \relates Period */
    Period operator*(Integer n, TimeUnit units);
    /*! \relates Period */
    Period operator*(TimeUnit units, Integer n);

    /*! \relates Period */
    bool operator<(const Period&, const Period&);
    /*! \relates Period */
    bool operator==(const Period&, const Period&);
    /*! \relates Period */
    bool operator!=(const Period&, const Period&);
    /*! \relates Period */
    bool operator>(const Period&, const Period&);
    /*! \relates Period */
    bool operator<=(const Period&, const Period&);
    /*! \relates Period */
    bool operator>=(const Period&, const Period&);


    //! Concrete date class
    /*! This class provides methods to inspect dates as well as methods and
        operators which implement a limited date algebra (increasing and
        decreasing dates, and calculating their difference).

        \ingroup datetime

        \test self-consistency of dates, serial numbers, days of
              month, months, and weekdays is checked over the whole
              date range.
    */
    class Date {
      public:
        //! \name constructors
        //@{
        //! Default constructor returning a null date.
        Date();
        //! Constructor taking a serial number as given by Applix or Excel.
        explicit Date(BigInteger serialNumber);
        //! More traditional constructor.
        Date(Day d, Month m, Year y);
        //@}
        //! \name inspectors
        //@{
        Weekday weekday() const;
        Day dayOfMonth() const;
        //! One-based (Jan 1st = 1)
        Day dayOfYear() const;
        Month month() const;
        Year year() const;
        BigInteger serialNumber() const;
        //@}

        //! \name date algebra
        //@{
        //! increments date by the given number of days
        Date& operator+=(BigInteger days);
        //! increments date by the given period
        Date& operator+=(const Period&);
        //! decrement date by the given number of days
        Date& operator-=(BigInteger days);
        //! decrements date by the given period
        Date& operator-=(const Period&);
        //! 1-day pre-increment
        Date& operator++();
        //! 1-day post-increment
        Date operator++(int );
        //! 1-day pre-decrement
        Date& operator--();
        //! 1-day post-decrement
        Date operator--(int );
        //! returns a new date incremented by the given number of days
        Date operator+(BigInteger days) const;
        //! returns a new date incremented by the given period
        Date operator+(const Period&) const;
        //! returns a new date decremented by the given number of days
        Date operator-(BigInteger days) const;
        //! returns a new date decremented by the given period
        Date operator-(const Period&) const;
        //@}

        //! \name static methods
        //@{
        //! today's date.
        static Date todaysDate();
        //! earliest allowed date
        static Date minDate();
        //! latest allowed date
        static Date maxDate();
        //! whether the given year is a leap one
        static bool isLeap(Year y);
        //! last day of the month to which the given date belongs
        static Date endOfMonth(const Date& d);
        //! whether a date is the last day of its month
        static bool isEOM(const Date& d);
        //! next given weekday following or equal to the given date
        /*! E.g., the Friday following January 15th, 20 (a Tuesday)
            was January 18th, 2002.

            see http://www.cpearson.com/excel/DateTimeWS.htm
        */
        static Date nextWeekday(const Date& d, Weekday);
        //! n-th given weekday in the given month and year
        /*! E.g., the 4th Thursday of March, 1998 was March 26th,
            1998.

            see http://www.cpearson.com/excel/DateTimeWS.htm
        */
        static Date nthWeekday(Size n, Weekday, Month m, Year y);
        //! whether or not the given date is an IMM date
        static bool isIMMdate(const Date& d);
        //! next IMM date following (or equal to) the given date
        /*! returns the 1st delivery date for next contract listed in the
            International Money Market section of the Chicago Mercantile
            Exchange.

            \warning The result date is following or equal to the original date
        */
        static Date nextIMMdate(const Date& d);
        //@}
      private:
        BigInteger serialNumber_;
        static Date advance(const Date& d, Integer units, TimeUnit);
        static Integer monthLength(Month m, bool leapYear);
        static Integer monthOffset(Month m, bool leapYear);
        static BigInteger yearOffset(Year y);
        static BigInteger minimumSerialNumber();
        static BigInteger maximumSerialNumber();
    };

    /*! \relates Date
        \brief Difference in days between dates
    */
    BigInteger operator-(const Date&, const Date&);

    /*! \relates Date */
    bool operator==(const Date&, const Date&);
    /*! \relates Date */
    bool operator!=(const Date&, const Date&);
    /*! \relates Date */
    bool operator<(const Date&, const Date&);
    /*! \relates Date */
    bool operator<=(const Date&, const Date&);
    /*! \relates Date */
    bool operator>(const Date&, const Date&);
    /*! \relates Date */
    bool operator>=(const Date&, const Date&);
    /*! \relates Date */
    std::ostream& operator<<(std::ostream&, const Date&);

    namespace detail {

        struct short_date_holder {
            short_date_holder(const Date d) : d(d) {}
            Date d;
        };
        std::ostream& operator<<(std::ostream&, const short_date_holder&);

        struct long_date_holder {
            long_date_holder(const Date& d) : d(d) {}
            Date d;
        };
        std::ostream& operator<<(std::ostream&, const long_date_holder&);

        struct iso_date_holder {
            iso_date_holder(const Date& d) : d(d) {}
            Date d;
        };
        std::ostream& operator<<(std::ostream&, const iso_date_holder&);

    }

    namespace io {

        //! output dates in short format (mm/dd/yyyy)
        /*! \ingroup manips */
        detail::short_date_holder short_date(const Date&);

        //! output dates in long format (Month ddth, yyyy)
        /*! \ingroup manips */
        detail::long_date_holder long_date(const Date&);

        //! output dates in ISO format (yyyy-mm-dd)
        /*! \ingroup manips */
        detail::iso_date_holder iso_date(const Date&);

    }


    #ifndef QL_DISABLE_DEPRECATED
    //! Formats dates for output
    /*! \deprecated use streams and manipulators for proper formatting */
    class DateFormatter {
      public:
        enum Format { Long, Short, ISO };
        static std::string toString(const Date& d,
                                    Format f = Long);
    };

    //! Formats weekday for output
    /*! \deprecated use streams and manipulators for proper formatting */
    class WeekdayFormatter {
      public:
        enum Format { Long, Short, Shortest };
        static std::string toString(Weekday wd,
                                    Format f = Long);
    };

    //! Formats frequency for output
    /*! \deprecated use streams and manipulators for proper formatting */
    class FrequencyFormatter {
      public:
        static std::string toString(Frequency f);
    };
    #endif


    // inline definitions

    inline Period operator*(Integer n, TimeUnit units) {
        return Period(n,units);
    }

    inline Period operator*(TimeUnit units, Integer n) {
        return Period(n,units);
    }

    inline bool operator==(const Period& p1, const Period& p2) {
        return !(p1 < p2 || p2 < p1);
    }

    inline bool operator!=(const Period& p1, const Period& p2) {
        return !(p1 == p2);
    }

    inline bool operator>(const Period& p1, const Period& p2) {
        return p2 < p1;
    }

    inline bool operator<=(const Period& p1, const Period& p2) {
        return !(p1 > p2);
    }

    inline bool operator>=(const Period& p1, const Period& p2) {
        return !(p1 < p2);
    }


    inline Weekday Date::weekday() const {
        Integer w = serialNumber_ % 7;
        return Weekday(w == 0 ? 7 : w);
    }

    inline Day Date::dayOfMonth() const {
        return dayOfYear() - monthOffset(month(),isLeap(year()));
    }

    inline Day Date::dayOfYear() const {
        return serialNumber_ - yearOffset(year());
    }

    inline BigInteger Date::serialNumber() const {
        return serialNumber_;
    }

    inline Date Date::operator+(BigInteger days) const {
        return Date(serialNumber_+days);
    }

    inline Date Date::operator-(BigInteger days) const {
        return Date(serialNumber_-days);
    }

    inline Date Date::operator+(const Period& p) const {
        return advance(*this,p.length(),p.units());
    }

    inline Date Date::operator-(const Period& p) const {
        return advance(*this,-p.length(),p.units());
    }

    inline Date Date::endOfMonth(const Date& d) {
        Month m = d.month();
        Year y = d.year();
        return Date(monthLength(m, isLeap(y)), m, y);
    }

    inline bool Date::isEOM(const Date& d) {
       return (d.dayOfMonth() == monthLength(d.month(), isLeap(d.year())));
    }

    inline bool Date::isIMMdate(const Date& date) {
        Day d = date.dayOfMonth();
        Month m = date.month();
        return ((date.weekday() == Wednesday) && (d >= 15 && d <= 21) &&
                (m == March || m == June || m == September || m == December));
    }

    inline BigInteger operator-(const Date& d1, const Date& d2) {
        return d1.serialNumber()-d2.serialNumber();
    }

    inline bool operator==(const Date& d1, const Date& d2) {
        return (d1.serialNumber() == d2.serialNumber());
    }

    inline bool operator!=(const Date& d1, const Date& d2) {
        return (d1.serialNumber() != d2.serialNumber());
    }

    inline bool operator<(const Date& d1, const Date& d2) {
        return (d1.serialNumber() < d2.serialNumber());
    }

    inline bool operator<=(const Date& d1, const Date& d2) {
        return (d1.serialNumber() <= d2.serialNumber());
    }

    inline bool operator>(const Date& d1, const Date& d2) {
        return (d1.serialNumber() > d2.serialNumber());
    }

    inline bool operator>=(const Date& d1, const Date& d2) {
        return (d1.serialNumber() >= d2.serialNumber());
    }

}


#endif
