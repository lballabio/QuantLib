
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

/*! \file date.h
    \brief date- and time-related classes, typedefs and enumerations 
    
    $Source$
    $Name$
    $Log$
    Revision 1.6  2001/01/04 20:09:31  nando
    few changes: enumerations, tab/spaces, more checks, python test, bug fixed

    Revision 1.5  2000/12/14 12:32:29  lballabio
    Added CVS tags in Doxygen file documentation blocks
    
*/

#ifndef quantlib_date_h
#define quantlib_date_h

#include "qldefines.h"
#include "qlerrors.h"

namespace QuantLib {

    //! Continuous quantity with 1-year units
    typedef double Time;
    
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

    typedef int Year;
    
    //! Units used to describe time periods
    enum TimeUnit { Days   = 0,
                    Weeks  = 1,
                    Months = 2,
                    Years  = 3 };
    
    
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
        static const int minimumSerialNumber, maximumSerialNumber;
        static const Date MinimumDate, MaximumDate;
        static const Day monthOffsetStorage[], monthLeapOffsetStorage[], 
            monthLengthStorage[], monthLeapLengthStorage[];
        static const Day QL_PTR_CONST monthOffset;
        static const Day QL_PTR_CONST monthLeapOffset;
        static const Day QL_PTR_CONST monthLength;
        static const Day QL_PTR_CONST monthLeapLength;
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
