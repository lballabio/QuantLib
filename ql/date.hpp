
/*
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

#ifndef quantlib_date_h
#define quantlib_date_h

#include <ql/errors.hpp>
#include <ql/types.hpp>
#include <utility>
#include <functional>

namespace QuantLib {

    //! Day number
    typedef int Day;

    /*! Day's serial number MOD 7;
    WEEKDAY Excel function is the same except for Sunday = 7
    */
    enum Weekday { Sunday    = 1,
                   Monday    = 2,
                   Tuesday   = 3,
                   Wednesday = 4,
                   Thursday  = 5,
                   Friday    = 6,
                   Saturday  = 7};

    //! Month names
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
                 December  = 12 };

    //! Year number
    typedef int Year;

    //! Frequency of events
    enum Frequency { Once = 0,    //!< e.g., a zero-coupon
                     Annual = 1,
                     Semiannual = 2,
                     Quarterly = 4,
                     Bimonthly = 6,
                     Monthly = 12 };

    //! Units used to describe time periods
    enum TimeUnit { Days   = 0,
                    Weeks  = 1,
                    Months = 2,
                    Years  = 3 };

    //! Time period described by a number of a given time unit
    class Period {
      public:
        Period()
        : length_(0), units_(Days) {}
        Period(int n, TimeUnit units)
        : length_(n), units_(units) {}
        int length() const { return length_; }
        TimeUnit units() const { return units_; }
      private:
        int length_;
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
    */
    class Date {
      public:
        //! \name constructors
        //@{
        //! Default constructor returning a null date.
        Date();
        //! Constructor taking a serial number as given by Applix or Excel.
        explicit Date(long serialNumber);
        //! More traditional constructor.
        Date(Day d, Month m, Year y);

        //! \name inspectors
        //@{
        Weekday weekday() const;
        Day dayOfMonth() const;
        bool isEndOfMonth() const;
        Day lastDayOfMonth() const;
        //! One-based (Jan 1st = 1)
        Day dayOfYear() const;
        Month month() const;
        Year year() const;
        long serialNumber() const;
        //@}

        //! \name date algebra
        //@{
        //! increments date in place
        Date& operator+=(int days);
        //! decrement date in place
        Date& operator-=(int days);
        //! 1-day pre-increment
        Date& operator++();
        //! 1-day post-increment
        Date operator++(int );
        //! 1-day pre-decrement
        Date& operator--();
        //! 1-day post-decrement
        Date operator--(int );
        //! returns a new incremented date
        Date operator+(int days) const;
        //! returns a new decremented date
        Date operator-(int days) const;
        //@}

        //! \name other methods to increment/decrement dates
        //@{
        Date plusDays(int days) const;
        Date plusWeeks(int weeks) const;
        Date plusMonths(int months) const;
        Date plusYears(int years) const;
        Date plus(int units, TimeUnit) const;
        Date plus(const Period&) const;
        //@}

        //! \name static methods
        //@{
        static bool isLeap(Year y);
        //! earliest allowed date
        static Date minDate();
        //! latest allowed date
        static Date maxDate();
        //! today's date.
        static Date todaysDate();
        //@}
      private:
        long serialNumber_;
        static int monthLength(Month m, bool leapYear);
        static int monthOffset(Month m, bool leapYear);
        static long yearOffset(Year y);
        static long minimumSerialNumber();
        static long maximumSerialNumber();
    };

    /*! \relates Date
        \brief Difference in days between dates
    */
    long operator-(const Date&, const Date&);

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

    // inline definitions

    inline Weekday Date::weekday() const {
        int w = serialNumber_ % 7;
        return Weekday(w == 0 ? 7 : w);
    }

    inline Day Date::dayOfMonth() const {
        return dayOfYear() - monthOffset(month(),isLeap(year()));
    }

    inline bool Date::isEndOfMonth() const {
       return (dayOfMonth() == monthLength(month(), isLeap(year())));
    }

    inline Day Date::lastDayOfMonth() const {
       return monthLength(month(), isLeap(year()));
    }

    inline Day Date::dayOfYear() const {
        return serialNumber_ - yearOffset(year());
    }

    inline long Date::serialNumber() const {
        return serialNumber_;
    }

    inline Date Date::operator+(int days) const {
        return Date(serialNumber_+days);
    }

    inline Date Date::operator-(int days) const {
        return Date(serialNumber_-days);
    }

    inline Date Date::plusDays(int days) const {
        return Date(serialNumber_+days);
    }

    inline Date Date::plusWeeks(int weeks) const {
        return Date(serialNumber_+weeks*7);
    }

    inline Date Date::plus(const Period& p) const {
        return plus(p.length(),p.units());
    }

    inline long operator-(const Date& d1, const Date& d2) {
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
