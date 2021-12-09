/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2014 Peter Caspers

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

#include <ql/experimental/volatility/noarbsabrsmilesection.hpp>
#include <ql/pricingengines/blackformula.hpp>
#include <ql/termstructures/volatility/sabr.hpp>
#include <utility>


namespace QuantLib {

    NoArbSabrSmileSection::NoArbSabrSmileSection(Time timeToExpiry,
                                                 Rate forward,
                                                 std::vector<Real> sabrParams,
                                                 Real shift,
                                                 VolatilityType volatilityType)
    : SmileSection(timeToExpiry, DayCounter(), volatilityType), forward_(forward), params_(std::move(sabrParams)),
      shift_(shift) {
        init();
    }

    NoArbSabrSmileSection::NoArbSabrSmileSection(
        const Date& d, Rate forward, std::vector<Real> sabrParams, const DayCounter& dc, Real shift, VolatilityType volatilityType)
    : SmileSection(d, dc, Date(), volatilityType), forward_(forward), params_(std::move(sabrParams)),
      shift_(shift) {
        init();
    }

void NoArbSabrSmileSection::init() {
    QL_REQUIRE(params_.size() >= 4,
               "sabr expects 4 parameters (alpha,beta,nu,rho) but ("
                   << params_.size() << ") given");
    QL_REQUIRE(forward_ > 0.0, "forward (" << forward_ << ") must be positive");
    QL_REQUIRE(
        shift_ == 0.0,
        "shift (" << shift_
                  << ") must be zero, other shifts are not implemented yet");
    model_ =
        ext::make_shared<NoArbSabrModel>(exerciseTime(), forward_, params_[0],
                                           params_[1], params_[2], params_[3]);
}

Real NoArbSabrSmileSection::optionPrice(Rate strike, Option::Type type,
                                        Real discount) const {
    Real call = model_->optionPrice(strike);
    return discount *
           (type == Option::Call ? call : call - (forward_ - strike));
}

Real NoArbSabrSmileSection::digitalOptionPrice(Rate strike, Option::Type type,
                                               Real discount, Real) const {
    Real call = model_->digitalOptionPrice(strike);
    return discount * (type == Option::Call ? call : 1.0 - call);
}

Real NoArbSabrSmileSection::density(Rate strike, Real discount, Real) const {
    return discount * model_->density(strike);
}

Real NoArbSabrSmileSection::volatilityImpl(Rate strike) const {

    Real impliedVol = 0.0;
    try {
        Option::Type type;
        if (strike >= forward_)
            type = Option::Call;
        else
            type = Option::Put;
        impliedVol =
            blackFormulaImpliedStdDev(type, strike, forward_,
                                      optionPrice(strike, type, 1.0), 1.0) /
            std::sqrt(exerciseTime());
    } catch (...) {
    }
    if (impliedVol == 0.0)
        // fall back on Hagan 2002 expansion
        impliedVol = 
            unsafeSabrVolatility(strike, forward_, exerciseTime(), params_[0],
                                 params_[1], params_[2], params_[3], volatilityType());

    return impliedVol;
}
} // namespace QuantLib
