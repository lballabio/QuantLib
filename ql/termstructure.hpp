/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004, 2005, 2006 StatPro Italia srl

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

/*! \file termstructure.hpp
    \brief base class for term structures
*/

#ifndef quantlib_term_structure_hpp
#define quantlib_term_structure_hpp

#include <ql/time/calendar.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>
#include <ql/settings.hpp>
#include <ql/handle.hpp>
#include <ql/math/interpolations/extrapolation.hpp>
#include <ql/utilities/null.hpp>

namespace QuantLib {

    //! Basic term-structure functionality
    class TermStructure : public virtual Observer,
                          public virtual Observable,
                          public Extrapolator {
      public:
        /*! \name Constructors

            There are three ways in which a term structure can keep
            track of its reference date.  The first is that such date
            is fixed; the second is that it is determined by advancing
            the current date of a given number of business days; and
            the third is that it is based on the reference date of
            some other structure.

            In the first case, the constructor taking a date is to be
            used; the default implementation of referenceDate() will
            then return such date. In the second case, the constructor
            taking a number of days and a calendar is to be used;
            referenceDate() will return a date calculated based on the
            current evaluation date, and the term structure and its
            observers will be notified when the evaluation date
            changes. In the last case, the referenceDate() method must
            be overridden in derived classes so that it fetches and
            return the appropriate date.
        */
        //@{
        //! default constructor
        /*! \warning term structures initialized by means of this
                     constructor must manage their own reference date
                     by overriding the referenceDate() method.
        */
        TermStructure(const DayCounter& dc = Actual365Fixed());
        //! initialize with a fixed reference date
        TermStructure(const Date& referenceDate,
                      const Calendar& calendar = Calendar(),
                      const DayCounter& dc = Actual365Fixed());
        //! calculate the reference date based on the global evaluation date
        TermStructure(Natural settlementDays,
                      const Calendar&,
                      const DayCounter& dc = Actual365Fixed());
        //@}
        virtual ~TermStructure() {}
        //! \name Dates
        //@{
        //! the day counter used for date/time conversion
        virtual DayCounter dayCounter() const;
        //! the latest date for which the curve can return values
        virtual Date maxDate() const = 0;
        //! the latest time for which the curve can return values
        virtual Time maxTime() const { return timeFromReference(maxDate()); }
        //! the date at which discount = 1.0 and/or variance = 0.0
        virtual const Date& referenceDate() const;
        //! the calendar used for reference date calculation
        virtual Calendar calendar() const;
        //! the settlementDays used for reference date calculation
        virtual Natural settlementDays() const;
        //@}

        //! \name Observer interface
        //@{
        void update();
        //@}
      protected:
        //! date/time conversion
        Time timeFromReference(const Date& date) const;
        //! date-range check
        void checkRange(const Date&,
                        bool extrapolate) const;
        //! time-range check
        void checkRange(Time,
                        bool extrapolate) const;
        bool moving_;
      private:
        mutable Date referenceDate_;
        mutable bool updated_;
        Natural settlementDays_;
        Calendar calendar_;
        DayCounter dayCounter_;
    };

    // inline definitions

    inline TermStructure::TermStructure(const DayCounter& dc)
    : moving_(false), updated_(true), settlementDays_(Null<Size>()),
      dayCounter_(dc) {}

    inline TermStructure::TermStructure(const Date& referenceDate,
                                        const Calendar& cal,
                                        const DayCounter& dc)
    : moving_(false), referenceDate_(referenceDate), updated_(true),
      settlementDays_(Null<Natural>()), calendar_(cal), dayCounter_(dc) {}

    inline TermStructure::TermStructure(Natural settlementDays,
                                        const Calendar& cal,
                                        const DayCounter& dc)
    : moving_(true), updated_(false), settlementDays_(settlementDays),
      calendar_(cal), dayCounter_(dc) {
        registerWith(Settings::instance().evaluationDate());
        // verify immediately if calendar and settlementDays are ok
        Date today = Settings::instance().evaluationDate();
        referenceDate_ = calendar().advance(today, settlementDays_, Days);
    }

    inline const Date& TermStructure::referenceDate() const {
        if (!updated_) {
            Date today = Settings::instance().evaluationDate();
            referenceDate_ = calendar().advance(today, settlementDays_, Days);
            updated_ = true;
        }
        return referenceDate_;
    }

    inline Calendar TermStructure::calendar() const {
        return calendar_;
    }

    inline DayCounter TermStructure::dayCounter() const {
        return dayCounter_;
    }
    
    inline Natural TermStructure::settlementDays() const {
        return settlementDays_;
    }

    inline void TermStructure::update() {
        if (moving_)
            updated_ = false;
        notifyObservers();
    }

    inline Time TermStructure::timeFromReference(const Date& d) const {
        return dayCounter().yearFraction(referenceDate(),d);
    }

    inline void TermStructure::checkRange(const Date& d,
                                          bool extrapolate) const {
        checkRange(timeFromReference(d),extrapolate);
    }

    inline void TermStructure::checkRange(Time t, bool extrapolate) const {
        QL_REQUIRE(t >= 0.0,
                   "negative time (" << t << ") given");
        QL_REQUIRE(extrapolate || allowsExtrapolation() || t <= maxTime(),
                   "time (" << t << ") is past max curve time ("
                            << maxTime() << ")");
    }

}

#endif
