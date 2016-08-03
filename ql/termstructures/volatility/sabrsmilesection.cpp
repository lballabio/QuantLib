/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Mario Pucci
 Copyright (C) 2015 Peter Caspers

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

#include <ql/termstructures/volatility/sabrsmilesection.hpp>
#include <ql/termstructures/volatility/sabr.hpp>
#include <ql/utilities/dataformatters.hpp>

namespace QuantLib {

    SabrSmileSection::SabrSmileSection(Time timeToExpiry,
                                       Rate forward,
                                       const std::vector<Real>& sabrParams,
                                       const Real shift)
        : SmileSection(timeToExpiry,DayCounter(),
                       ShiftedLognormal,shift),
          forward_(forward), shift_(shift) {

        alpha_ = sabrParams[0];
        beta_ = sabrParams[1];
        nu_ = sabrParams[2];
        rho_ = sabrParams[3];

        QL_REQUIRE(forward_ + shift_ > 0.0,
                   "at the money forward rate + shift must be "
                   "positive: "
                       << io::rate(forward_) << " with shift "
                       << io::rate(shift_) << " not allowed");
        validateSabrParameters(alpha_, beta_, nu_, rho_);
    }

    SabrSmileSection::SabrSmileSection(const Date& d,
                                       Rate forward,
                                       const std::vector<Real>& sabrParams,
                                       const DayCounter& dc,
                                       const Real shift)
        : SmileSection(d, dc,Date(),ShiftedLognormal,shift),
          forward_(forward), shift_(shift) {

        alpha_ = sabrParams[0];
        beta_ = sabrParams[1];
        nu_ = sabrParams[2];
        rho_ = sabrParams[3];

        QL_REQUIRE(forward_ + shift_ > 0.0,
                   "at the money forward rate + shift must be "
                   "positive: "
                       << io::rate(forward_) << " with shift "
                       << io::rate(shift_) << " not allowed");
        validateSabrParameters(alpha_, beta_, nu_, rho_);
    }

     Real SabrSmileSection::varianceImpl(Rate strike) const {
        strike = std::max(0.00001 - shift(),strike);
        Volatility vol = unsafeShiftedSabrVolatility(
            strike, forward_, exerciseTime(), alpha_, beta_, nu_, rho_, shift_);
        return vol * vol * exerciseTime();
     }

     Real SabrSmileSection::volatilityImpl(Rate strike) const {
        strike = std::max(0.00001 - shift(),strike);
        return unsafeShiftedSabrVolatility(strike, forward_, exerciseTime(),
                                           alpha_, beta_, nu_, rho_, shift_);
     }
}
