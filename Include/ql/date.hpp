
/*
 * Copyright (C) 2000-2001 QuantLib Group
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
 * The license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.sourceforge.net/Authors.txt
*/

/*! \file date.hpp
    \brief date- and time-related classes, typedefs and enumerations

    $Id$
*/

// $Source$
// $Log$
// Revision 1.6  2001/07/05 15:57:22  lballabio
// Collected typedefs in a single file
//
// Revision 1.5  2001/06/22 16:38:15  lballabio
// Improved documentation
//
// Revision 1.4  2001/06/15 13:52:06  lballabio
// Reworked indexes
//
// Revision 1.3  2001/05/24 15:38:07  nando
// smoothing #include xx.hpp and cutting old Log messages
//

#ifndef quantlib_date_h
#define quantlib_date_h

#include "ql/errors.hpp"
#include "ql/types.hpp"
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
    operators which implement a limited date algebra (increasing and decreasing
    dates, and calculating their difference).
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
        Date operator++(int);
        //! 1-day pre-decrement
        Date& operator--();
        //! 1-day post-decrement
        Date operator--(int);
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
        static const int minimumSerialNumber;
        static const int maximumSerialNumber;
        static const Date MinimumDate;
        static const Date MaximumDate;
        static const Day  monthOffsetStorage[];
        static const Day  monthLeapOffsetStorage[];
        static const Day  monthLengthStorage[];
        static const Day  monthLeapLengthStorage[];
        static const Day* monthOffset;
        static const Day* monthLeapOffset;
        static const Day* monthLength;
        static const Day* monthLeapLength;
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

}


#endif
