/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2026

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

/*! \file gaussian2dnonstandardswaptionengine.hpp
    \brief Two-factor Gaussian model engine for callable capped/floored
           floating-rate bonds with stochastic credit spread.

    Extends the Gaussian1dNonstandardSwaptionEngine to two correlated
    Gaussian factors: one for the risk-free rate and one for the credit
    spread. The coupon forecasting uses the rate factor only, while
    discounting uses the combined rate + spread.

    Backward induction is performed on a 2D standardized state grid.
    The correlation between factors is handled via Cholesky decomposition
    in the integration step: the bivariate Gaussian integral is factored
    into two nested 1D integrals.

    Cap/floor on floating coupons is applied at each grid node using
    the model-implied forward rate, ensuring state-dependent moneyness.

    \ingroup swaptionengines
*/

#ifndef quantlib_gaussian2d_nonstandard_swaption_engine_hpp
#define quantlib_gaussian2d_nonstandard_swaption_engine_hpp

#include <ql/instruments/nonstandardswaption.hpp>
#include <ql/models/shortrate/twofactormodels/gaussian2dmodel.hpp>
#include <ql/pricingengines/genericmodelengine.hpp>

namespace QuantLib {

class Gaussian2dNonstandardSwaptionEngine
    : public GenericModelEngine<Gaussian2dModel,
                                NonstandardSwaption::arguments,
                                NonstandardSwaption::results> {
  public:
    Gaussian2dNonstandardSwaptionEngine(
        const ext::shared_ptr<Gaussian2dModel>& model,
        int integrationPoints = 32,
        Real stddevs = 7.0,
        bool extrapolatePayoff = true,
        bool flatPayoffExtrapolation = false)
        : GenericModelEngine<Gaussian2dModel,
                             NonstandardSwaption::arguments,
                             NonstandardSwaption::results>(model),
          integrationPoints_(integrationPoints),
          stddevs_(stddevs),
          extrapolatePayoff_(extrapolatePayoff),
          flatPayoffExtrapolation_(flatPayoffExtrapolation) {}

    void calculate() const override;

  private:
    /*! Compute the NPV of the underlying swap at exercise date expiry,
        conditional on rate state yRate and spread state ySpread.
        Coupon forecasting uses yRate only (via forecastZerobond).
        Discounting uses both (via discountZerobond).
        Cap/floor is applied in rate space using the model forward rate. */
    Real underlyingNpv(const Date& expiry, Real yRate, Real ySpread) const;

    /*! Perform 1D Gaussian polynomial integration of a cubic spline.
        Integrates payoff(z) * φ(z) over the z grid with optional
        extrapolation. Returns the integral value. */
    Real gaussianIntegral1d(const Array& z, const Array& values,
                            bool isCall) const;

    const int integrationPoints_;
    const Real stddevs_;
    const bool extrapolatePayoff_;
    const bool flatPayoffExtrapolation_;
};

}

#endif
