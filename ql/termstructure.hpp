
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.org/
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
 * if not, please email quantlib-users@lists.sourceforge.net
 * The license is also available at http://quantlib.org/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.org/group.html
*/

/*! \file termstructure.hpp
    \brief Term structure

    \fullpath
    ql/%termstructure.hpp
*/

// $Id$

#ifndef quantlib_term_structure_h
#define quantlib_term_structure_h

#include "ql/calendar.hpp"
#include "ql/currency.hpp"
#include "ql/daycounter.hpp"
#include "ql/relinkablehandle.hpp"
#include <vector>

/*! \namespace QuantLib::TermStructures
    \brief Concrete implementations of the TermStructure interface

    See sect. \ref termstructures
*/

namespace QuantLib {

    //! Term structure
    /*! This abstract class defines the interface of concrete
        rate structures which will be derived from this one.
    */
    class TermStructure : public Patterns::Observable {
      public:
        virtual ~TermStructure() {}
        //! \name Rates and discount
        //@{
        //! zero yield rate at a given date
        Rate zeroYield(const Date&, bool extrapolate = false) const;
        //! zero yield rate at a given time from settlement
        Rate zeroYield(Time, bool extrapolate = false) const;
        //! discount factor at a given date
        DiscountFactor discount(const Date&, bool extrapolate = false) const;
        //! discount factor at a given time from settlement
        DiscountFactor discount(Time, bool extrapolate = false) const;
        //! instantaneous forward rate at a given date
        Rate forward(const Date&, bool extrapolate = false) const;
        //! instantaneous forward rate at a given time from settlement
        Rate forward(Time, bool extrapolate = false) const;
        //@}

        //! \name Dates
        //@{
        //! returns today's date
        virtual Date todaysDate() const = 0;
        //! returns the number of settlement days
        virtual int settlementDays() const = 0;
        //! returns the calendar for settlement calculation
        virtual Handle<Calendar> calendar() const = 0;
        //! returns the day counter
        virtual Handle<DayCounter> dayCounter() const = 0;

        //! returns the settlement date
        virtual Date settlementDate() const = 0;
        //! returns the earliest date for which the curve can return rates
        virtual Date minDate() const = 0;
        //! returns the latest date for which the curve can return rates
        virtual Date maxDate() const = 0;
        //! returns the earliest time for which the curve can return rates
        virtual Time minTime() const = 0;
        //! returns the latest date for which the curve can return rates
        virtual Time maxTime() const = 0;
        //@}

        //! \name Other inspectors
        //@{
        //! returns the currency upon which the term structure is defined
        virtual Currency currency() const = 0;
        //@}
      protected:
        //! implements the actual zero yield calculation in derived classes
        virtual Rate zeroYieldImpl(Time,
            bool extrapolate = false) const = 0;
        //! implements the actual discount calculation in derived classes
        virtual DiscountFactor discountImpl(Time,
            bool extrapolate = false) const = 0;
        //! implements the actual forward rate calculation in derived classes
        virtual Rate forwardImpl(Time,
            bool extrapolate = false) const = 0;
    };

    //! Zero yield term structure
    /*! This abstract class acts as an adapter to TermStructure allowing the
        programmer to implement only the <tt>zeroYield(Time)</tt> method in 
        derived classes.
    */
    class ZeroYieldStructure : public TermStructure {
      public:
        virtual ~ZeroYieldStructure() {}
      protected:
        /*! Returns the discount factor for the given date calculating it 
            from the zero yield.
        */
        DiscountFactor discountImpl(Time, bool extrapolate = false) const;
        /*! Returns the instantaneous forward rate for the given date
            calculating it from the zero yield.
        */
        Rate forwardImpl(Time, bool extrapolate = false) const;
    };

    //! Discount factor term structure
    /*! This abstract class acts as an adapter to TermStructure allowing the
        programmer to implement only the <tt>discount(const Date&)</tt> 
        method in derived classes.
    */
    class DiscountStructure : public TermStructure {
      public:
        virtual ~DiscountStructure() {}
      protected:
        /*! Returns the zero yield rate for the given date calculating it 
            from the discount.
        */
        Rate zeroYieldImpl(Time, bool extrapolate = false) const;
        /*! Returns the instantaneous forward rate for the given date
            calculating it from the discount.
        */
        Rate forwardImpl(Time, bool extrapolate = false) const;
    };

    //! Forward rate term structure
    /*! This abstract class acts as an adapter to TermStructure allowing the
        programmer to implement only the <tt>forward(const Date&)</tt> method
        in derived classes.
    */
    class ForwardRateStructure : public TermStructure {
      public:
        virtual ~ForwardRateStructure() {}
      protected:
        /*! Returns the zero yield rate for the given date calculating it 
            from the instantaneous forward rate.
            \warning This is just a default, highly inefficient 
                implementation. Derived classes should implement their own 
                zeroYield method.
        */
        Rate zeroYieldImpl(Time, bool extrapolate = false) const;
        /*! Returns the discount factor for the given date calculating it 
            from the instantaneous forward rate.
        */
        DiscountFactor discountImpl(Time, bool extrapolate = false) const;
    };

    //! Implied term structure at a given date in the future
    /*! The given date will be the implied today's date.
        \note This term structure will remain linked to the original 
            structure, i.e., any changes in the latter will be reflected in 
            this structure as well.
    */
    class ImpliedTermStructure : public DiscountStructure,
                                 public Patterns::Observer {
      public:
        ImpliedTermStructure(const RelinkableHandle<TermStructure>&,
            const Date& todaysDate);
        ~ImpliedTermStructure();
        //! \name TermStructure interface
        //@{
        Currency currency() const;
        Date todaysDate() const;
        int settlementDays() const;
        Handle<Calendar> calendar() const;
        Handle<DayCounter> dayCounter() const;
        Date settlementDate() const;
        Date maxDate() const;
        Date minDate() const;
        Time maxTime() const;
        Time minTime() const;
        //@}
        //! \name Observer interface
        //@{
        void update();
        //@}
      protected:
        //! returns the discount factor as seen from the evaluation date
        DiscountFactor discountImpl(Time, bool extrapolate = false) const;
      private:
        RelinkableHandle<TermStructure> originalCurve_;
        Date todaysDate_;
    };

    //! Term structure with an added spread on the zero yield rate
    /*! \note This term structure will remain linked to the original 
            structure, i.e., any changes in the latter will be reflected in 
            this structure as well.
    */
    class SpreadedTermStructure : public ZeroYieldStructure,
                                  public Patterns::Observer {
      public:
        SpreadedTermStructure(const RelinkableHandle<TermStructure>&, 
            Spread spread);
        ~SpreadedTermStructure();
        //! \name TermStructure interface
        //@{
        Currency currency() const;
        Date todaysDate() const;
        int settlementDays() const;
        Handle<Calendar> calendar() const;
        Handle<DayCounter> dayCounter() const;
        Date settlementDate() const;
        Date maxDate() const;
        Date minDate() const;
        Time maxTime() const;
        Time minTime() const;
        //@}
        //! \name Observer interface
        //@{
        void update();
        //@}
      protected:
        //! returns the spreaded zero yield rate
        Rate zeroYieldImpl(Time, bool extrapolate = false) const;
      private:
        RelinkableHandle<TermStructure> originalCurve_;
        Spread spread_;
    };


    // inline definitions

    inline Rate TermStructure::zeroYield(const Date& d,
        bool extrapolate) const {
            Time t = dayCounter()->yearFraction(settlementDate(),d);
            return zeroYieldImpl(t,extrapolate);
    }

    inline Rate TermStructure::zeroYield(Time t, bool extrapolate) const {
        return zeroYieldImpl(t,extrapolate);
    }
        
    inline DiscountFactor TermStructure::discount(const Date& d,
        bool extrapolate) const {
            Time t = dayCounter()->yearFraction(settlementDate(),d);
            return discountImpl(t,extrapolate);
    }

    inline DiscountFactor TermStructure::discount(Time t, 
        bool extrapolate) const {
            return discountImpl(t,extrapolate);
    }

    inline Rate TermStructure::forward(const Date& d,
        bool extrapolate) const {
            Time t = dayCounter()->yearFraction(settlementDate(),d);
            return forwardImpl(t,extrapolate);
    }

    inline Rate TermStructure::forward(Time t, bool extrapolate) const {
        return forwardImpl(t,extrapolate);
    }


    // curve deriving discount and forward from zero yield

    inline DiscountFactor ZeroYieldStructure::discountImpl(Time t,
        bool extrapolate) const {
            Rate r = zeroYield(t, extrapolate);
            return DiscountFactor(QL_EXP(-r*t));
    }

    inline Rate ZeroYieldStructure::forwardImpl(Time t,
        bool extrapolate) const {
            Time dt = 0.001;
            Rate r1 = zeroYield(t, extrapolate), 
                 r2 = zeroYield(t+dt, true);
            return r1+t*(r2-r1)/dt;
    }


    // curve deriving zero yield and forward from discount

    inline Rate DiscountStructure::zeroYieldImpl(Time t,
        bool extrapolate) const {
            DiscountFactor f = discount(t, extrapolate);
            return Rate(-QL_LOG(f)/t);
    }

    inline Rate DiscountStructure::forwardImpl(Time t,
        bool extrapolate) const {
            Time dt = 0.001;
            DiscountFactor f1 = discount(t, extrapolate),
                           f2 = discount(t+dt, true);
            return Rate(QL_LOG(f1/f2)/dt);
    }


    // curve deriving zero yield and discount from forward

    inline Rate ForwardRateStructure::zeroYieldImpl(Time t,
        bool extrapolate) const {
            if (t == 0.0)
                return forward(0.0);
            double sum = 0.5*forward(0.0);
            size_t N = 1000;
            double dt = t/N;
            for (Time i=t+dt; i<t; i+=dt)
                sum += forward(i, extrapolate);
            sum += 0.5*forward(t, extrapolate);
            return Rate(sum*dt/t);
    }

    inline DiscountFactor ForwardRateStructure::discountImpl(Time t,
        bool extrapolate) const {
            Rate r = zeroYield(t, extrapolate);
            return DiscountFactor(QL_EXP(-r*t));
    }



    // time-shifted curve

    inline ImpliedTermStructure::ImpliedTermStructure(
        const RelinkableHandle<TermStructure>& h, const Date& todaysDate)
    : originalCurve_(h), todaysDate_(todaysDate) {
        if (!originalCurve_.isNull())
            originalCurve_.registerObserver(this);
    }

    inline ImpliedTermStructure::~ImpliedTermStructure() {
        if (!originalCurve_.isNull())
            originalCurve_.unregisterObserver(this);
    }

    inline Currency ImpliedTermStructure::currency() const {
        return originalCurve_->currency();
    }

    inline Date ImpliedTermStructure::todaysDate() const {
        return todaysDate_;
    }

    inline int ImpliedTermStructure::settlementDays() const {
        return originalCurve_->settlementDays();
    }

    inline Handle<Calendar> ImpliedTermStructure::calendar() const {
        return originalCurve_->calendar();
    }

    inline Handle<DayCounter> ImpliedTermStructure::dayCounter() const {
        return originalCurve_->dayCounter();
    }

    inline Date ImpliedTermStructure::settlementDate() const {
        return calendar()->advance(
            todaysDate_,settlementDays(),Days);
    }

    inline Date ImpliedTermStructure::maxDate() const {
        return originalCurve_->maxDate();
    }

    inline Date ImpliedTermStructure::minDate() const {
        return settlementDate();
    }

    inline Time ImpliedTermStructure::maxTime() const {
        return dayCounter()->yearFraction(
            settlementDate(),originalCurve_->maxDate());
    }

    inline Time ImpliedTermStructure::minTime() const {
        return 0.0;
    }

    inline void ImpliedTermStructure::update() {
        notifyObservers();
    }

    inline DiscountFactor ImpliedTermStructure::discountImpl(Time t,
        bool extrapolate) const {
            /* t is relative to the current settlement date
               and needs to be converted to the time relative
               to the settlement date of the original curve */
            Time originalTime = t + dayCounter()->yearFraction(
                originalCurve_->settlementDate(),settlementDate());
            // evaluationDate cannot be an extrapolation
            /* discount at evaluation date cannot be cached
               since the original curve could change between
               invocations of this method */
            return originalCurve_->discount(originalTime, extrapolate) / 
                   originalCurve_->discount(settlementDate(),false);
    }


    // spreaded curve

    inline SpreadedTermStructure::SpreadedTermStructure(
        const RelinkableHandle<TermStructure>& h, Spread spread)
    : originalCurve_(h), spread_(spread) {
        if (!originalCurve_.isNull())
            originalCurve_.registerObserver(this);
    }

    inline SpreadedTermStructure::~SpreadedTermStructure() {
        if (!originalCurve_.isNull())
            originalCurve_.unregisterObserver(this);
    }

    inline Currency SpreadedTermStructure::currency() const {
        return originalCurve_->currency();
    }

    inline Date SpreadedTermStructure::todaysDate() const {
        return originalCurve_->todaysDate();
    }

    inline int SpreadedTermStructure::settlementDays() const {
        return originalCurve_->settlementDays();
    }

    inline Handle<Calendar> SpreadedTermStructure::calendar() const {
        return originalCurve_->calendar();
    }

    inline Handle<DayCounter> SpreadedTermStructure::dayCounter() const {
        return originalCurve_->dayCounter();
    }

    inline Date SpreadedTermStructure::settlementDate() const {
        return originalCurve_->settlementDate();
    }

    inline Date SpreadedTermStructure::maxDate() const {
        return originalCurve_->maxDate();
    }

    inline Date SpreadedTermStructure::minDate() const {
        return originalCurve_->minDate();
    }

    inline Time SpreadedTermStructure::maxTime() const {
        return originalCurve_->maxTime();
    }

    inline Time SpreadedTermStructure::minTime() const {
        return originalCurve_->minTime();
    }

    inline void SpreadedTermStructure::update() {
        notifyObservers();
    }

    inline Rate SpreadedTermStructure::zeroYieldImpl(Time t,
        bool extrapolate) const {
            return originalCurve_->zeroYield(t, extrapolate) + spread_;
    }

}


#endif
