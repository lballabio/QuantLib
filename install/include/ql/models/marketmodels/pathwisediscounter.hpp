/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Mark Joshi

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


#ifndef quantlib_pathwise_market_model_discounter_hpp
#define quantlib_pathwise_market_model_discounter_hpp

#include <ql/types.hpp>
#include <ql/math/matrix.hpp>
#include <vector>

namespace QuantLib {

    class CurveState;
    /*!
    this class returns the number of units of the discretely compounding money
    market account that 1 unit of cash at the payment can buy using the LIBOR
    rates from current step.

    It also returns the derivative of this number with respect to each of the
    rates.

    Discounting is purely based on the simulation LIBOR rates,
    to get a discounting back to zero you need to multiply by
    the discount factor of t_0.
    */
    class MarketModelPathwiseDiscounter {
      public:
        MarketModelPathwiseDiscounter(Time paymentTime,
                                      const std::vector<Time>& rateTimes);
        void getFactors(const Matrix& LIBORRates, // LIBOR rate values for all steps
                        const Matrix& Discounts, // P(t_0, t_j) for j=0,...n for each step
                        Size currentStep,
                        std::vector<Real>& factors) const;
      private:
        Size before_;
        Size numberRates_;
        Real beforeWeight_;
        Real postWeight_;
        std::vector<Real> taus_;

    };

}

#endif
