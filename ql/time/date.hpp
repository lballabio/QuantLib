/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005, 2006 StatPro Italia srl
 Copyright (C) 2004, 2005, 2006 Ferdinando Ametrano
 Copyright (C) 2006 Katiuscia Manzoni
 Copyright (C) 2006 Toyin Akin
 Copyright (C) 2015 Klaus Spanderen
 Copyright (C) 2020 Leonardo Arcari
 Copyright (C) 2020 Kline s.r.l.

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

/*! \file date.hpp
    \brief date- and time-related classes, typedefs and enumerations
*/

#ifndef quantlib_date_hpp
#define quantlib_date_hpp

#include <ql/time/period.hpp>
#include <ql/time/weekday.hpp>
#include <ql/utilities/null.hpp>
#include <boost/cstdint.hpp>

#ifdef QL_HIGH_RESOLUTION_DATE
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/posix_time_duration.hpp>
#endif

#include <utility>
#include <functional>
#include <string>


namespace QuantLib {

    //! Day number
    /*! \ingroup datetime */
    typedef Integer Day;

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

    //! Year number
    /*! \ingroup datetime */
    typedef Integer Year;

#ifdef QL_HIGH_RESOLUTION_DATE
    //! Hour number
    /*! \ingroup datetime */
    typedef boost::posix_time::hours::hour_type Hour;

    //! Minute number
    /*! \ingroup datetime */
    typedef boost::posix_time::minutes::min_type Minute;

    //! Second number
    /*! \ingroup datetime */
    typedef boost::posix_time::minutes::sec_type Second;

    //! Millisecond number
    /*! \ingroup datetime */
    typedef boost::posix_time::time_duration::fractional_seconds_type
        Millisecond;

    //! Millisecond number
    /*! \ingroup datetime */
    typedef boost::posix_time::time_duration::fractional_seconds_type
        Microsecond;
#endif

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
        //! serial number type
        typedef boost::int_fast32_t serial_type;
        //! \name constructors
        //@{
        //! Default constructor returning a null date.
        Date();
        //! Constructor taking a serial number as given by Applix or Excel.
        explicit Date(Date::serial_type serialNumber);
        //! More traditional constructor.
        Date(Day d, Month m, Year y);

#ifdef QL_HIGH_RESOLUTION_DATE
        //! Constructor taking boost posix date time object
        explicit Date(const boost::posix_time::ptime& localTime);
        //! More traditional constructor.
        Date(Day d, Month m, Year y,
             Hour hours, Minute minutes, Second seconds,
             Millisecond millisec = 0, Microsecond microsec = 0);
#endif
        //@}

        //! \name inspectors
        //@{
        Weekday weekday() const;
        Day dayOfMonth() const;
        //! One-based (Jan 1st = 1)
        Day dayOfYear() const;
        Month month() const;
        Year year() const;
        Date::serial_type serialNumber() const;

#ifdef QL_HIGH_RESOLUTION_DATE
        Hour hours() const;
        Minute minutes() const;
        Second seconds() const;
        Millisecond milliseconds() const;
        Microsecond microseconds() const;

        Time fractionOfDay() const;
        Time fractionOfSecond() const;

        const boost::posix_time::ptime& dateTime() const;
#endif
        //@}

        //! \name date algebra
        //@{
        //! increments date by the given number of days
        Date& operator+=(Date::serial_type days);
        //! increments date by the given period
        Date& operator+=(const Period&);
        //! decrement date by the given number of days
        Date& operator-=(Date::serial_type days);
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
        Date operator+(Date::serial_type days) const;
        //! returns a new date incremented by the given period
        Date operator+(const Period&) const;
        //! returns a new date decremented by the given number of days
        Date operator-(Date::serial_type days) const;
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
        static bool isEndOfMonth(const Date& d);
        //! next given weekday following or equal to the given date
        /*! E.g., the Friday following Tuesday, January 15th, 2002
            was January 18th, 2002.

            see http://www.cpearson.com/excel/DateTimeWS.htm
        */
        static Date nextWeekday(const Date& d,
                                Weekday w);
        //! n-th given weekday in the given month and year
        /*! E.g., the 4th Thursday of March, 1998 was March 26th,
            1998.

            see http://www.cpearson.com/excel/DateTimeWS.htm
        */
        static Date nthWeekday(Size n,
                               Weekday w,
                               Month m,
                               Year y);

#ifdef QL_HIGH_RESOLUTION_DATE
        //! local date time, based on the time zone settings of the computer
        static Date localDateTime();
        //! UTC date time
        static Date universalDateTime();

        //! underlying resolution of the  posix date time object
        static boost::posix_time::time_duration::tick_type ticksPerSecond();
#endif

        //@}

      private:
        static Date::serial_type minimumSerialNumber();
        static Date::serial_type maximumSerialNumber();
        static void checkSerialNumber(Date::serial_type serialNumber);

#ifdef QL_HIGH_RESOLUTION_DATE
        boost::posix_time::ptime dateTime_;
#else
        Date::serial_type serialNumber_;
        static Date advance(const Date& d, Integer units, TimeUnit);
        static Integer monthLength(Month m, bool leapYear);
        static Integer monthOffset(Month m, bool leapYear);
        static Date::serial_type yearOffset(Year y);
#endif
    };

    /*! \relates Date
        \brief Difference in days between dates
    */
    Date::serial_type operator-(const Date&, const Date&);
    /*! \relates Date
        \brief Difference in days (including fraction of days) between dates
    */
    Time daysBetween(const Date&, const Date&);

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

    /*!
      Compute a hash value of @p d.

      This method makes Date hashable via <tt>boost::hash</tt>.

      Example:

      \code{.cpp}
      #include <boost/unordered_set.hpp>

      boost::unordered_set<Date> set;
      Date d = Date(1, Jan, 2020); 

      set.insert(d);
      assert(set.count(d)); // 'd' was added to 'set'
      \endcode

      \param [in] d Date to hash
      \return A hash value of @p d
      \relates Date
    */
    std::size_t hash_value(const Date& d);

    /*! \relates Date */
    std::ostream& operator<<(std::ostream&, const Date&);

    namespace detail {

        struct short_date_holder {
            explicit short_date_holder(const Date d) : d(d) {}
            Date d;
        };
        std::ostream& operator<<(std::ostream&, const short_date_holder&);

        struct long_date_holder {
            explicit long_date_holder(const Date& d) : d(d) {}
            Date d;
        };
        std::ostream& operator<<(std::ostream&, const long_date_holder&);

        struct iso_date_holder {
            explicit iso_date_holder(const Date& d) : d(d) {}
            Date d;
        };
        std::ostream& operator<<(std::ostream&, const iso_date_holder&);

        struct formatted_date_holder {
            formatted_date_holder(const Date& d, const std::string& f)
            : d(d), f(f) {}
            Date d;
            std::string f;
        };
        std::ostream& operator<<(std::ostream&,
                                 const formatted_date_holder&);

#ifdef QL_HIGH_RESOLUTION_DATE
        struct iso_datetime_holder {
            explicit iso_datetime_holder(const Date& d) : d(d) {}
            Date d;
        };
        std::ostream& operator<<(std::ostream&, const iso_datetime_holder&);
#endif
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

        //! output dates in user defined format using boost date functionality
        /*! \ingroup manips */
        detail::formatted_date_holder formatted_date(const Date&,
                                                     const std::string& fmt);

#ifdef QL_HIGH_RESOLUTION_DATE
        //! output datetimes in ISO format (YYYY-MM-DDThh:mm:ss,SSSSSS)
        /*! \ingroup manips */
        detail::iso_datetime_holder iso_datetime(const Date&);
#endif

    }

    //! specialization of Null template for the Date class
    template <>
    class Null<Date> {
      public:
        Null() {}
        operator Date() const { return Date(); }
    };


#ifndef QL_HIGH_RESOLUTION_DATE
    // inline definitions

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

    inline Date::serial_type Date::serialNumber() const {
        return serialNumber_;
    }

    inline Date Date::operator+(Date::serial_type days) const {
        return Date(serialNumber_+days);
    }

    inline Date Date::operator-(Date::serial_type days) const {
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

    inline bool Date::isEndOfMonth(const Date& d) {
       return (d.dayOfMonth() == monthLength(d.month(), isLeap(d.year())));
    }

    inline Date::serial_type operator-(const Date& d1, const Date& d2) {
        return d1.serialNumber()-d2.serialNumber();
    }

    inline Time daysBetween(const Date& d1, const Date& d2) {
        return Time(d2-d1);
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
#endif
}

#endif
