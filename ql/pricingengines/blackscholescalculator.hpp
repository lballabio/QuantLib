/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Ferdinando Ametrano

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

/*! \file blackscholescalculator.hpp
    \brief Black-Scholes formula calculator class
*/

#ifndef quantlib_blackscholescalculator_hpp
#define quantlib_blackscholescalculator_hpp

#include <ql/pricingengines/blackcalculator.hpp>

namespace QuantLib {

    //! Black-Scholes 1973 calculator class
    class BlackScholesCalculator : public BlackCalculator {
      public:
        BlackScholesCalculator(
                        const ext::shared_ptr<StrikedTypePayoff>& payoff,
                        Real spot,
                        DiscountFactor growth,
                        Real stdDev,
                        DiscountFactor discount);
        BlackScholesCalculator(Option::Type optionType,
                               Real strike,
                               Real spot,
                               DiscountFactor growth,
                               Real stdDev,
                               DiscountFactor discount);
        ~BlackScholesCalculator() override = default;
        /*! Sensitivity to change in the underlying spot price. */
        Real delta() const;
        /*! Sensitivity in percent to a percent change in the
            underlying spot price. */
        Real elasticity() const;
        /*! Second order derivative with respect to change in the
            underlying spot price. */
        Real gamma() const;
        /*! Sensitivity to time to maturity. */
        Real theta(Time maturity) const;
        /*! Sensitivity to time to maturity per day
            (assuming 365 day in a year). */
        Real thetaPerDay(Time maturity) const;
        // also un-hide overloads taking a spot
        using BlackCalculator::delta;
        using BlackCalculator::elasticity;
        using BlackCalculator::gamma;
        using BlackCalculator::theta;
        using BlackCalculator::thetaPerDay;
      protected:
        Real spot_;
        DiscountFactor growth_;
    };

    // inline
    inline Real BlackScholesCalculator::delta() const {
        return BlackCalculator::delta(spot_);
    }

    inline Real BlackScholesCalculator::elasticity() const {
        return BlackCalculator::elasticity(spot_);
    }

    inline Real BlackScholesCalculator::gamma() const {
        return BlackCalculator::gamma(spot_);
    }

    inline Real BlackScholesCalculator::theta(Time maturity) const {
        return BlackCalculator::theta(spot_, maturity);
    }

    inline Real BlackScholesCalculator::thetaPerDay(Time maturity) const {
        return BlackCalculator::thetaPerDay(spot_, maturity);
    }

}

#endif


#ifndef id_f86a27a595031c2cfb156e5bfedbd123
#define id_f86a27a595031c2cfb156e5bfedbd123
inline bool test_f86a27a595031c2cfb156e5bfedbd123(const int* i) {
    return i != nullptr;
}
#endif
