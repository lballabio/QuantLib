
/*
 Copyright (C) 2000, 2001, 2002 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file date.hpp
    \brief date- and time-related classes, typedefs and enumerations

    \fullpath
    ql/%date.hpp
*/

// $Id$

#ifndef quantlib_date_h
#define quantlib_date_h

#include <ql/errors.hpp>
#include <ql/types.hpp>
#include <utility>

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
        explicit Date(int serialNumber);
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
        int serialNumber() const;
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
        //@}
      private:
        int serialNumber_;
        static Size monthLength(Month m, bool leapYear);
        static Size monthOffset(Month m, bool leapYear);
        static Size yearOffset(Year y);

    };

    /*! \relates Date
        \brief Difference in days between dates
    */
    int operator-(const Date&, const Date&);

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
    std::ostream& operator<< (std::ostream& stream, const Date& result);


    // inline definitions
    
    inline Weekday Date::weekday() const {
        int w = serialNumber_ % 7;
        return Weekday(w == 0 ? 7 : w);
    }

    inline Day Date::dayOfMonth() const {
        return dayOfYear() - monthOffset(month(),isLeap(year()));
    }

    inline Day Date::dayOfYear() const {
        return serialNumber_ - yearOffset(year());
    }

    inline int Date::serialNumber() const {
        return serialNumber_;
    }

    inline Date& Date::operator+=(int days) {
        serialNumber_ += days;
        return *this;
    }

    inline Date& Date::operator-=(int days) {
        serialNumber_ -= days;
        return *this;
    }

    inline Date& Date::operator++() {
        serialNumber_++;
        return *this;
    }

    inline Date Date::operator++(int ) {
        Date temp = *this;
        serialNumber_++;
        return temp;
    }

    inline Date& Date::operator--() {
        serialNumber_--;
        return *this;
    }

    inline Date Date::operator--(int ) {
        Date temp = *this;
        serialNumber_--;
        return temp;
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

    inline int operator-(const Date& d1, const Date& d2) {
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
