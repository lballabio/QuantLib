
/*
 Copyright (C) 2000, 2001, 2002 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software developed by the QuantLib Group; you can
 redistribute it and/or modify it under the terms of the QuantLib License;
 either version 1.0, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 QuantLib License for more details.

 You should have received a copy of the QuantLib License along with this
 program; if not, please email ferdinando@ametrano.net

 The QuantLib License is also available at http://quantlib.org/license.html
 The members of the QuantLib Group are listed in the QuantLib License
*/
/*! \file daycounter.hpp
    \brief day counter class

    \fullpath
    ql/%daycounter.hpp
*/

// $Id$

#ifndef quantlib_day_counter_h
#define quantlib_day_counter_h

#include <ql/date.hpp>
#include <ql/handle.hpp>
#include <ql/null.hpp>

/*! \namespace QuantLib::DayCounters
    \brief Specialized DayCounter classes

    See sect. \ref dayconters
*/

namespace QuantLib {

    //! day counter class
    /*! This class provides methods for determining the length of a time
        period according to given market convention, both as a number
        of days and as a year fraction.

        The Strategy pattern is used to provide the base behavior of the
        day counter.
    */
    class DayCounter {
      public:
        //! \name DayCounter interface
        //@{
        //! Returns the name of the day counter.
        /*! \warning This method is used for output and comparison between
                day counters. It is <b>not</b> meant to be used for writing
                switch-on-type code.
        */
        std::string name() const;
        //! Returns the number of days between two dates.
        int dayCount(const Date&, const Date&) const;
        //! Returns the period between two dates as a fraction of year.
        Time yearFraction(const Date&, const Date&,
          const Date& refPeriodStart = Date(),
          const Date& refPeriodEnd = Date()) const;
        //@}

        //! abstract base class for calendar factories
        class DayCounterFactory {
          public:
            virtual DayCounter create() const = 0;
        };
        typedef DayCounterFactory factory;

        //! abstract base class for day counter implementations
        class DayCounterImpl {
          public:
            virtual std::string name() const = 0;
            virtual int dayCount(const Date&, const Date&) const = 0;
            virtual Time yearFraction(const Date&, const Date&,
                const Date& refPeriodStart,
                const Date& refPeriodEnd) const = 0;
        };
      protected:
        /*! this protected constructor will only be invoked by derived
            classes which define a given Calendar implementation */
        DayCounter(const Handle<DayCounterImpl>& impl) : impl_(impl) {}
      private:
        Handle<DayCounterImpl> impl_;
    };

    // comparison based on name

    /*! Returns <tt>true</tt> iff the two day counters belong to the same
        derived class.
        \relates DayCounter
    */
    bool operator==(const DayCounter&, const DayCounter&);

    /*! \relates DayCounter */
    bool operator!=(const DayCounter&, const DayCounter&);


    // inline definitions

    inline std::string DayCounter::name() const {
        return impl_->name();
    }

    inline int DayCounter::dayCount(const Date& d1, const Date& d2) const {
        return impl_->dayCount(d1,d2);
    }

    inline Time DayCounter::yearFraction(const Date& d1, const Date& d2,
        const Date& refPeriodStart, const Date& refPeriodEnd) const {
            return impl_->yearFraction(d1,d2,refPeriodStart,refPeriodEnd);
    }


    inline bool operator==(const DayCounter& h1, const DayCounter& h2) {
        return (h1.name() == h2.name());
    }

    inline bool operator!=(const DayCounter& h1, const DayCounter& h2) {
        return (h1.name() != h2.name());
    }

}


#endif
