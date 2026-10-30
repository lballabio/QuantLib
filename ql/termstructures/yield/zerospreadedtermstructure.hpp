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
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file zerospreadedtermstructure.hpp
    \brief Zero spreaded term structure
*/

#ifndef quantlib_zero_spreaded_term_structure_hpp
#define quantlib_zero_spreaded_term_structure_hpp

#include <ql/quote.hpp>
#include <ql/termstructures/yield/derivedtermstructure.hpp>
#include <ql/termstructures/yield/zeroyieldstructure.hpp>
#include <utility>

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
    class ZeroSpreadedTermStructure
        : public RelativeDerivedYieldTermStructure<ZeroYieldStructure> {
      public:
        ZeroSpreadedTermStructure(Handle<YieldTermStructure>,
                                  Handle<Quote> spread,
                                  Compounding comp = Continuous,
                                  Frequency freq = NoFrequency);

        /*! \deprecated Use the constructor without a day counter.
                        Deprecated in version 1.41.
        */
        [[deprecated("Use the constructor without DayCounter")]]
        ZeroSpreadedTermStructure(Handle<YieldTermStructure>,
                                  Handle<Quote> spread,
                                  Compounding comp,
                                  Frequency freq,
                                  const DayCounter& dc);
      protected:
        //! returns the spreaded zero yield rate
        Rate zeroYieldImpl(Time) const override;
      private:
        Handle<Quote> spread_;
        Compounding comp_;
        Frequency freq_;
    };

    inline ZeroSpreadedTermStructure::ZeroSpreadedTermStructure(Handle<YieldTermStructure> h,
                                                                Handle<Quote> spread,
                                                                Compounding comp,
                                                                Frequency freq)
    : RelativeDerivedYieldTermStructure(std::move(h)), spread_(std::move(spread)),
      comp_(comp), freq_(freq) {
        registerWith(spread_);
    }

    inline ZeroSpreadedTermStructure::ZeroSpreadedTermStructure(Handle<YieldTermStructure> h,
                                                                Handle<Quote> spread,
                                                                Compounding comp,
                                                                Frequency freq,
                                                                const DayCounter& dc)
    : ZeroSpreadedTermStructure(std::move(h), std::move(spread), comp, freq) {}

    inline Rate ZeroSpreadedTermStructure::zeroYieldImpl(Time t) const {
        InterestRate zeroRate =
            originalCurve_->zeroRate(t, comp_, freq_, true);
        InterestRate spreadedRate(zeroRate + spread_->value(),
                                  zeroRate.dayCounter(),
                                  zeroRate.compounding(),
                                  zeroRate.frequency());
        return spreadedRate.equivalentRate(Continuous, NoFrequency, t);
    }

}

#endif
