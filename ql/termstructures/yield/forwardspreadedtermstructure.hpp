/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2008 StatPro Italia srl

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

/*! \file forwardspreadedtermstructure.hpp
    \brief Forward-spreaded term structure
*/

#ifndef quantlib_forward_spreaded_term_structure_hpp
#define quantlib_forward_spreaded_term_structure_hpp

#include <ql/termstructures/yield/forwardstructure.hpp>
#include <ql/quote.hpp>

namespace QuantLib {

    //! Term structure with added spread on the instantaneous forward rate
    /*! \note This term structure will remain linked to the original
              structure, i.e., any changes in the latter will be
              reflected in this structure as well.

        \ingroup yieldtermstructures

        \test
        - the correctness of the returned values is tested by
          checking them against numerical calculations.
        - observability against changes in the underlying term
          structure and in the added spread is checked.
    */
    class ForwardSpreadedTermStructure : public ForwardRateStructure {
      public:
        ForwardSpreadedTermStructure(const Handle<YieldTermStructure>&,
                                     const Handle<Quote>& spread);
        //! \name TermStructure interface
        //@{
        DayCounter dayCounter() const;
        Date maxDate() const;
        Time maxTime() const;
        const Date& referenceDate() const;
        Calendar calendar() const;
        Natural settlementDays() const;
        //@}
        //! \name Observer interface
        //@{
        void update();
        //@}
      protected:
        //! \name ForwardRateStructure implementation
        //@{
        Rate forwardImpl(Time t) const;
        /* This method must disappear should the spread become a curve */
        Rate zeroYieldImpl(Time t) const;
        //@}
      private:
        Handle<YieldTermStructure> originalCurve_;
        Handle<Quote> spread_;
    };

    inline ForwardSpreadedTermStructure::ForwardSpreadedTermStructure(
                                          const Handle<YieldTermStructure>& h,
                                          const Handle<Quote>& spread)
    : originalCurve_(h), spread_(spread) {
        registerWith(originalCurve_);
        registerWith(spread_);
    }

    inline DayCounter ForwardSpreadedTermStructure::dayCounter() const {
        return originalCurve_->dayCounter();
    }

    inline Calendar ForwardSpreadedTermStructure::calendar() const {
        return originalCurve_->calendar();
    }

    inline Natural ForwardSpreadedTermStructure::settlementDays() const {
        return originalCurve_->settlementDays();
    }

    inline const Date& ForwardSpreadedTermStructure::referenceDate() const {
        return originalCurve_->referenceDate();
    }

    inline Date ForwardSpreadedTermStructure::maxDate() const {
        return originalCurve_->maxDate();
    }

    inline Time ForwardSpreadedTermStructure::maxTime() const {
        return originalCurve_->maxTime();
    }

    inline void ForwardSpreadedTermStructure::update() {
        if (!originalCurve_.empty()) {
            YieldTermStructure::update();
        } else {
            /* The implementation inherited from YieldTermStructure
               asks for our reference date, which we don't have since
               the original curve is still not set. Therefore, we skip
               over that and just call the base-class behavior. */
            TermStructure::update();
        }
    }

    inline Rate ForwardSpreadedTermStructure::forwardImpl(Time t) const {
        return originalCurve_->forwardRate(t, t, Continuous, NoFrequency, true)
            + spread_->value();
    }

    inline Rate ForwardSpreadedTermStructure::zeroYieldImpl(Time t) const {
        return originalCurve_->zeroRate(t, Continuous, NoFrequency, true)
            + spread_->value();
    }

}

#endif
