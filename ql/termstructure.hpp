

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
/*! \file termstructure.hpp
    \brief Term structure

    \fullpath
    ql/%termstructure.hpp
*/

// $Id$

#ifndef quantlib_term_structure_hpp
#define quantlib_term_structure_hpp

#include <ql/calendar.hpp>
#include <ql/currency.hpp>
#include <ql/daycounter.hpp>
#include <ql/marketelement.hpp>
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
        virtual Calendar calendar() const = 0;
        //! returns the day counter
        virtual DayCounter dayCounter() const = 0;

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
        Calendar calendar() const;
        DayCounter dayCounter() const;
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
    class ZeroSpreadedTermStructure : public ZeroYieldStructure,
                                  public Patterns::Observer {
      public:
        ZeroSpreadedTermStructure(const RelinkableHandle<TermStructure>&,
            const RelinkableHandle<MarketElement>& spread);
        ~ZeroSpreadedTermStructure();
        //! \name TermStructure interface
        //@{
        Currency currency() const;
        Date todaysDate() const;
        int settlementDays() const;
        Calendar calendar() const;
        DayCounter dayCounter() const;
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
        //! returns the spreaded forward rate
        /*! \warning This method must disappear should the spread become a curve */
        Rate forwardImpl(Time, bool extrapolate = false) const;
      private:
        RelinkableHandle<TermStructure> originalCurve_;
        RelinkableHandle<MarketElement> spread_;
    };



    //! Term structure with an added spread on the instantaneous forward rate
    /*! \note This term structure will remain linked to the original
            structure, i.e., any changes in the latter will be reflected in
            this structure as well.
    */
    class ForwardSpreadedTermStructure : public ForwardRateStructure,
                                         public Patterns::Observer {
      public:
        ForwardSpreadedTermStructure(const RelinkableHandle<TermStructure>&,
            const RelinkableHandle<MarketElement>& spread);
        ~ForwardSpreadedTermStructure();
        //! \name TermStructure interface
        //@{
        Currency currency() const;
        Date todaysDate() const;
        int settlementDays() const;
        Calendar calendar() const;
        DayCounter dayCounter() const;
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
        //! returns the spreaded forward rate
        Rate forwardImpl(Time, bool extrapolate = false) const;
        //! returns the spreaded zero yield rate
        /*! \warning This method must disappear should the spread become a curve */
        Rate zeroYieldImpl(Time, bool extrapolate = false) const;
      private:
        RelinkableHandle<TermStructure> originalCurve_;
        RelinkableHandle<MarketElement> spread_;
    };




    // inline definitions

    inline Rate TermStructure::zeroYield(const Date& d,
        bool extrapolate) const {
            Time t = dayCounter().yearFraction(settlementDate(),d);
            return zeroYieldImpl(t,extrapolate);
    }

    inline Rate TermStructure::zeroYield(Time t, bool extrapolate) const {
        return zeroYieldImpl(t,extrapolate);
    }

    inline DiscountFactor TermStructure::discount(const Date& d,
        bool extrapolate) const {
            Time t = dayCounter().yearFraction(settlementDate(),d);
            return discountImpl(t,extrapolate);
    }

    inline DiscountFactor TermStructure::discount(Time t,
        bool extrapolate) const {
            return discountImpl(t,extrapolate);
    }

    inline Rate TermStructure::forward(const Date& d,
        bool extrapolate) const {
            Time t = dayCounter().yearFraction(settlementDate(),d);
            return forwardImpl(t,extrapolate);
    }

    inline Rate TermStructure::forward(Time t, bool extrapolate) const {
        return forwardImpl(t,extrapolate);
    }


    // curve deriving discount and forward from zero yield

    inline DiscountFactor ZeroYieldStructure::discountImpl(Time t,
        bool extrapolate) const {
            Rate r = zeroYieldImpl(t, extrapolate);
            return DiscountFactor(QL_EXP(-r*t));
    }

    inline Rate ZeroYieldStructure::forwardImpl(Time t,
        bool extrapolate) const {
            Time dt = 0.001;
            Rate r1 = zeroYieldImpl(t, extrapolate),
                 r2 = zeroYieldImpl(t+dt, true);
            return r1+t*(r2-r1)/dt;
    }


    // curve deriving zero yield and forward from discount

    inline Rate DiscountStructure::zeroYieldImpl(Time t,
        bool extrapolate) const {
            DiscountFactor f = discountImpl(t, extrapolate);
            return Rate(-QL_LOG(f)/t);
    }

    inline Rate DiscountStructure::forwardImpl(Time t,
        bool extrapolate) const {
            Time dt = 0.001;
            DiscountFactor f1 = discountImpl(t, extrapolate),
                           f2 = discountImpl(t+dt, true);
            return Rate(QL_LOG(f1/f2)/dt);
    }


    // curve deriving zero yield and discount from forward

    inline Rate ForwardRateStructure::zeroYieldImpl(Time t,
        bool extrapolate) const {
            if (t == 0.0)
                return forwardImpl(0.0);
            double sum = 0.5*forwardImpl(0.0);
            Size N = 1000;
            double dt = t/N;
            for (Time i=dt; i<t; i+=dt)
                sum += forwardImpl(i, extrapolate);
            sum += 0.5*forwardImpl(t, extrapolate);
            return Rate(sum*dt/t);
    }

    inline DiscountFactor ForwardRateStructure::discountImpl(Time t,
        bool extrapolate) const {
            Rate r = zeroYieldImpl(t, extrapolate);
            return DiscountFactor(QL_EXP(-r*t));
    }



    // time-shifted curve

    inline ImpliedTermStructure::ImpliedTermStructure(
        const RelinkableHandle<TermStructure>& h, const Date& todaysDate)
    : originalCurve_(h), todaysDate_(todaysDate) {
        originalCurve_.registerObserver(this);
    }

    inline ImpliedTermStructure::~ImpliedTermStructure() {
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

    inline Calendar ImpliedTermStructure::calendar() const {
        return originalCurve_->calendar();
    }

    inline DayCounter ImpliedTermStructure::dayCounter() const {
        return originalCurve_->dayCounter();
    }

    inline Date ImpliedTermStructure::settlementDate() const {
        return calendar().advance(
            todaysDate_,settlementDays(),Days);
    }

    inline Date ImpliedTermStructure::maxDate() const {
        return originalCurve_->maxDate();
    }

    inline Date ImpliedTermStructure::minDate() const {
        return settlementDate();
    }

    inline Time ImpliedTermStructure::maxTime() const {
        return dayCounter().yearFraction(
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
            Time originalTime = t + dayCounter().yearFraction(
                originalCurve_->settlementDate(),settlementDate());
            // evaluationDate cannot be an extrapolation
            /* discount at evaluation date cannot be cached
               since the original curve could change between
               invocations of this method */
            return originalCurve_->discount(originalTime, extrapolate) /
                   originalCurve_->discount(settlementDate(),false);
    }


    // zero-yield spreaded curves
    inline ZeroSpreadedTermStructure::ZeroSpreadedTermStructure(
        const RelinkableHandle<TermStructure>& h, 
        const RelinkableHandle<MarketElement>& spread)
    : originalCurve_(h), spread_(spread) {
        originalCurve_.registerObserver(this);
        spread_.registerObserver(this);
    }

    inline ZeroSpreadedTermStructure::~ZeroSpreadedTermStructure() {
        originalCurve_.unregisterObserver(this);
        spread_.unregisterObserver(this);
    }

    inline Currency ZeroSpreadedTermStructure::currency() const {
        return originalCurve_->currency();
    }

    inline Date ZeroSpreadedTermStructure::todaysDate() const {
        return originalCurve_->todaysDate();
    }

    inline int ZeroSpreadedTermStructure::settlementDays() const {
        return originalCurve_->settlementDays();
    }

    inline Calendar ZeroSpreadedTermStructure::calendar() const {
        return originalCurve_->calendar();
    }

    inline DayCounter ZeroSpreadedTermStructure::dayCounter() const {
        return originalCurve_->dayCounter();
    }

    inline Date ZeroSpreadedTermStructure::settlementDate() const {
        return originalCurve_->settlementDate();
    }

    inline Date ZeroSpreadedTermStructure::maxDate() const {
        return originalCurve_->maxDate();
    }

    inline Date ZeroSpreadedTermStructure::minDate() const {
        return originalCurve_->minDate();
    }

    inline Time ZeroSpreadedTermStructure::maxTime() const {
        return originalCurve_->maxTime();
    }

    inline Time ZeroSpreadedTermStructure::minTime() const {
        return originalCurve_->minTime();
    }

    inline void ZeroSpreadedTermStructure::update() {
        notifyObservers();
    }

    inline Rate ZeroSpreadedTermStructure::zeroYieldImpl(Time t,
        bool extrapolate) const {
            return originalCurve_->zeroYield(t, extrapolate) + spread_->value();
    }

    inline Rate ZeroSpreadedTermStructure::forwardImpl(Time t,
        bool extrapolate) const {
            return originalCurve_->forward(t, extrapolate) + spread_->value();
    }


    // forward spreaded curves
    inline ForwardSpreadedTermStructure::ForwardSpreadedTermStructure(
        const RelinkableHandle<TermStructure>& h, 
        const RelinkableHandle<MarketElement>& spread)
    : originalCurve_(h), spread_(spread) {
        originalCurve_.registerObserver(this);
        spread_.registerObserver(this);
    }

    inline ForwardSpreadedTermStructure::~ForwardSpreadedTermStructure() {
        originalCurve_.unregisterObserver(this);
        spread_.unregisterObserver(this);
    }

    inline Currency ForwardSpreadedTermStructure::currency() const {
        return originalCurve_->currency();
    }

    inline Date ForwardSpreadedTermStructure::todaysDate() const {
        return originalCurve_->todaysDate();
    }

    inline int ForwardSpreadedTermStructure::settlementDays() const {
        return originalCurve_->settlementDays();
    }

    inline Calendar ForwardSpreadedTermStructure::calendar() const {
        return originalCurve_->calendar();
    }

    inline DayCounter ForwardSpreadedTermStructure::dayCounter() const {
        return originalCurve_->dayCounter();
    }

    inline Date ForwardSpreadedTermStructure::settlementDate() const {
        return originalCurve_->settlementDate();
    }

    inline Date ForwardSpreadedTermStructure::maxDate() const {
        return originalCurve_->maxDate();
    }

    inline Date ForwardSpreadedTermStructure::minDate() const {
        return originalCurve_->minDate();
    }

    inline Time ForwardSpreadedTermStructure::maxTime() const {
        return originalCurve_->maxTime();
    }

    inline Time ForwardSpreadedTermStructure::minTime() const {
        return originalCurve_->minTime();
    }

    inline void ForwardSpreadedTermStructure::update() {
        notifyObservers();
    }

    inline Rate ForwardSpreadedTermStructure::forwardImpl(Time t,
        bool extrapolate) const {
            return originalCurve_->forward(t, extrapolate) + spread_->value();
    }

    inline Rate ForwardSpreadedTermStructure::zeroYieldImpl(Time t,
        bool extrapolate) const {
            return originalCurve_->zeroYield(t, extrapolate) + spread_->value();
    }

}


#endif
