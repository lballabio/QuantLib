/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2026 Yang Ye

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

/*! \file lognormalvolatilityswapengine.hpp
    \brief Volatility-swap engine assuming lognormal realized variance
*/

#ifndef quantlib_lognormal_volatility_swap_engine_hpp
#define quantlib_lognormal_volatility_swap_engine_hpp

#include <ql/instruments/volatilityswap.hpp>
#include <ql/quote.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>

namespace QuantLib {

    //! Volatility-swap engine assuming lognormal realized variance
    /*! Prices a volatility swap from the fair variance strike
        \f$ K \f$ of a variance swap with the same dates and from the
        volatility \f$ s \f$ of the logarithm of the realized variance
        \f$ V \f$.

        The realized variance is assumed to be lognormal with mean
        \f$ K \f$,
        \f[
        V = K \exp\left( -\frac{s^2}{2} + s Z \right),
        \f]
        with \f$ Z \f$ a standard normal variable.  Then \f$ \sqrt{V} \f$
        is lognormal as well, and the fair volatility strike is
        \f[
        E[\sqrt{V}] = \sqrt{K} \, e^{-s^2/8}.
        \f]
        To first order in \f$ s^2 \f$ this is the approximation
        \f$ \sqrt{K} - \mathrm{Var}(V) / (8 K^{3/2}) \f$ of
        Brockhaus and Long (2000), since
        \f$ \mathrm{Var}(V) = K^2 (e^{s^2} - 1) \f$.
        The factor \f$ e^{-s^2/8} \f$ is the convexity adjustment of the
        volatility swap with respect to the square root of the variance
        strike; it reduces to 1 when \f$ s = 0 \f$.

        The lognormal law of the realized variance is an assumption of
        this engine, not a result derived from a model of the underlying;
        the log-volatility \f$ s \f$ must be supplied by the user (for
        instance, calibrated to variance-swap or variance-option
        quotes) and refers to the whole life of the swap.
        The discount factor to the maturity date is deterministic, i.e.,
        the realized variance is treated as independent of rates.

        The swap value is
        \f[
        \pm N \, B(T) \left( \sqrt{K} \, e^{-s^2/8} - K_{vol} \right),
        \f]
        with \f$ B(T) \f$ the discount factor to the maturity date
        (payment date), positive for the long side.

        \warning This engine does not manage seasoned volatility swaps:
                 the start date must not precede the reference date of the
                 discount curve.

        \ingroup forwardengines

        \test the fair strike is verified against the closed form, against
              the variance-swap strike when \f$ s = 0 \f$, and against a
              Monte Carlo simulation of the lognormal realized variance
    */
    class LognormalVolatilitySwapEngine : public VolatilitySwap::engine {
      public:
        /*! \param discountCurve        curve used to discount the payoff
            \param fairVarianceStrike   fair (annualized) variance strike
                                        for the life of the swap, for
                                        instance as returned by a variance
                                        swap
            \param varianceLogVolatility volatility of the logarithm of
                                        the realized variance over the life
                                        of the swap; non-negative
        */
        LognormalVolatilitySwapEngine(Handle<YieldTermStructure> discountCurve,
                                      Handle<Quote> fairVarianceStrike,
                                      Handle<Quote> varianceLogVolatility);
        void calculate() const override;

      private:
        Handle<YieldTermStructure> discountCurve_;
        Handle<Quote> fairVarianceStrike_;
        Handle<Quote> varianceLogVolatility_;
    };

}

#endif
