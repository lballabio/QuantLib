
/*
 Copyright (C) 2004 Ferdinando Ametrano
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

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
                   Saturday  = 7
    };

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
                 December  = 12
    };

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
    enum Frequency { Once = 0,          //!< only once, e.g., a zero-coupon
                     Annual = 1,        //!< once a year
                     Semiannual = 2,    //!< twice a year
                     Quarterly = 4,     //!< every third month
                     Bimonthly = 6,     //!< every second month
                     Monthly = 12       //!< once a month
    };

    //! Units used to describe time periods
    /*! \ingroup datetime */
    enum TimeUnit { Days   = 0,
                    Weeks  = 1,
                    Months = 2,
                    Years  = 3
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

        //! \name inspectors
        //@{
        Weekday weekday() const;
        Day dayOfMonth() const;
        bool isEndOfMonth() const;
        bool isIMMdate() const;
        Day lastDayOfMonth() const;
        //! One-based (Jan 1st = 1)
        Day dayOfYear() const;
        Month month() const;
        Year year() const;
        BigInteger serialNumber() const;
        /*! returns the 1st delivery date for next contract listed in the
            International Money Market section of the Chicago Mercantile
            Exchange.
            
            \warning The result date is following or equal to the original date
        */
        //! next IMM date
        Date nextIMMdate() const;
        //@}

        //! \name date algebra
        //@{
        //! increments date in place
        Date& operator+=(BigInteger days);
        //! decrement date in place
        Date& operator-=(BigInteger days);
        //! 1-day pre-increment
        Date& operator++();
        //! 1-day post-increment
        Date operator++(int );
        //! 1-day pre-decrement
        Date& operator--();
        //! 1-day post-decrement
        Date operator--(int );
        //! returns a new incremented date
        Date operator+(BigInteger days) const;
        //! returns a new decremented date
        Date operator-(BigInteger days) const;
        //@}

        //! \name other methods to increment/decrement dates
        //@{
        Date plusDays(Integer days) const;
        Date plusWeeks(Integer weeks) const;
        Date plusMonths(Integer months) const;
        Date plusYears(Integer years) const;
        Date plus(Integer units, TimeUnit) const;
        Date plus(const Period&) const;
        //@}

        //! \name static methods
        //@{
        static bool isLeap(Year y);
        //! earliest allowed date
        static Date minDate();
        //! latest allowed date
        static Date maxDate();
        /*! returns the date of the next day of week following (or equal to)
            a given date.
            (e.g. the Friday following Tuesday 15-Jan-2002 is 18-Jan-2002)

            see http://www.cpearson.com/excel/DateTimeWS.htm
        */
        static Date nextDayOfWeekAfterDate(const Date& d,
                                           Weekday dayOfWeek);
        /*! returns the date of Nth weekday in a given month and year
            (e.g. 26-March-98 for the 4th Thursday of March, 1998)

            see http://www.cpearson.com/excel/DateTimeWS.htm
        */
        static Date nthDayOfWeekForMonthAndYear(Size n,
                                                Weekday dayOfWeek,
                                                Month m,
                                                Year y);
        //! today's date.
        static Date todaysDate();
        //@}
      private:
        BigInteger serialNumber_;
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

    //! Formats dates for output
    /*! Formatting can be in short (mm/dd/yyyy)
        or long (Month ddth, yyyy) form.
    */
    class DateFormatter {
      public:
        enum Format { Long, Short, ISO };
        static std::string toString(const Date& d,
                                    Format f = Long);
    };


    //! Formats weekday for output
    /*! Formatting can be in Long (full name), Short (three letters),
        of VeryShort (two letters) form.
    */
    class WeekdayFormatter {
      public:
        enum Format { Long, Short, VeryShort };
        static std::string toString(Weekday wd,
                                    Format f = Long);
        static std::string toString(const Date& d,
                                    Format f = Long) {
            return WeekdayFormatter::toString(d.weekday(), f);
        }
    };

    // inline definitions

    inline Weekday Date::weekday() const {
        Integer w = serialNumber_ % 7;
        return Weekday(w == 0 ? 7 : w);
    }

    inline Day Date::dayOfMonth() const {
        return dayOfYear() - monthOffset(month(),isLeap(year()));
    }

    inline bool Date::isEndOfMonth() const {
       return (dayOfMonth() == monthLength(month(), isLeap(year())));
    }

    inline bool Date::isIMMdate() const {
        Day d = this->dayOfMonth();
        Month m = this->month();
        return ((this->weekday() == Wednesday) && (d >= 15 && d <= 21) && 
                (m == March || m == June || m == September || m == December));
    }

    inline Day Date::lastDayOfMonth() const {
       return monthLength(month(), isLeap(year()));
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

    inline Date Date::plusDays(Integer days) const {
        return Date(serialNumber_+days);
    }

    inline Date Date::plusWeeks(Integer weeks) const {
        return Date(serialNumber_+weeks*7);
    }

    inline Date Date::plus(const Period& p) const {
        return plus(p.length(),p.units());
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
