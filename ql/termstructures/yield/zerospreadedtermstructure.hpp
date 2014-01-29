/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2007, 2008 StatPro Italia srl

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

/*! \file zerospreadedtermstructure.hpp
    \brief Zero spreaded term structure
*/

#ifndef quantlib_zero_spreaded_term_structure_hpp
#define quantlib_zero_spreaded_term_structure_hpp

#include <ql/termstructures/yield/zeroyieldstructure.hpp>
#include <ql/quote.hpp>

namespace QuantLib {

    //! Term structure with an added spread on the zero yield rate
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
    class ZeroSpreadedTermStructure : public ZeroYieldStructure {
      public:
        ZeroSpreadedTermStructure(const Handle<YieldTermStructure>&,
                                  const Handle<Quote>& spread,
                                  Compounding comp = Continuous,
                                  Frequency freq = NoFrequency,
                                  const DayCounter& dc = DayCounter());
        //! \name YieldTermStructure interface
        //@{
        DayCounter dayCounter() const;
        Calendar calendar() const;
        Natural settlementDays() const;
        const Date& referenceDate() const;
        Date maxDate() const;
        Time maxTime() const;
        //@}
        //! \name Observer interface
        //@{
        void update();
        //@}
      protected:
        //! returns the spreaded zero yield rate
        Rate zeroYieldImpl(Time) const;
        //! returns the spreaded forward rate
        /* This method must disappear should the spread become a curve */
        Rate forwardImpl(Time) const;
      private:
        Handle<YieldTermStructure> originalCurve_;
        Handle<Quote> spread_;
        Compounding comp_;
        Frequency freq_;
        DayCounter dc_;
    };

    inline ZeroSpreadedTermStructure::ZeroSpreadedTermStructure(
                                          const Handle<YieldTermStructure>& h,
                                          const Handle<Quote>& spread,
                                          Compounding comp,
                                          Frequency freq,
                                          const DayCounter& dc)
    : originalCurve_(h), spread_(spread), comp_(comp), freq_(freq), dc_(dc) {
        //QL_REQUIRE(h->dayCounter()==dc_,
        //           "spread daycounter (" << dc_ <<
        //           ") must be the same of the curve to be spreaded (" <<
        //           originalCurve_->dayCounter() <<
        //           ")");
        registerWith(originalCurve_);
        registerWith(spread_);
    }

    inline DayCounter ZeroSpreadedTermStructure::dayCounter() const {
        return originalCurve_->dayCounter();
    }

    inline Calendar ZeroSpreadedTermStructure::calendar() const {
        return originalCurve_->calendar();
    }

    inline Natural ZeroSpreadedTermStructure::settlementDays() const {
        return originalCurve_->settlementDays();
    }

    inline const Date& ZeroSpreadedTermStructure::referenceDate() const {
        return originalCurve_->referenceDate();
    }

    inline Date ZeroSpreadedTermStructure::maxDate() const {
        return originalCurve_->maxDate();
    }

    inline Time ZeroSpreadedTermStructure::maxTime() const {
        return originalCurve_->maxTime();
    }

    inline void ZeroSpreadedTermStructure::update() {
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

    inline Rate ZeroSpreadedTermStructure::zeroYieldImpl(Time t) const {
        // to be fixed: user-defined daycounter should be used
        InterestRate zeroRate =
            originalCurve_->zeroRate(t, comp_, freq_, true);
        InterestRate spreadedRate(zeroRate + spread_->value(),
                                  zeroRate.dayCounter(),
                                  zeroRate.compounding(),
                                  zeroRate.frequency());
        return spreadedRate.equivalentRate(Continuous, NoFrequency, t);
    }

    inline Rate ZeroSpreadedTermStructure::forwardImpl(Time t) const {
        return originalCurve_->forwardRate(t, t, comp_, freq_, true)
            + spread_->value();
    }

}

#endif
