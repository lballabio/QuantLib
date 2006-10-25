/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Mario Pucci

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/



#include <ql/Volatilities/smilesection.hpp>
#include <ql/Math/linearinterpolation.hpp>
#include <ql/Math/sabrinterpolation.hpp>

namespace QuantLib {


     InterpolatedSmileSection::InterpolatedSmileSection(Time timeToExpiry,
                                const std::vector<Rate>& strikes,
                                const std::vector<Rate>& volatilities)
     : timeToExpiry_(timeToExpiry), strikes_(strikes),
       volatilities_(volatilities) {

        interpolation_ = boost::shared_ptr<Interpolation>(new
            //SABRInterpolation(strikes_.begin(), strikes_.end(), volatilities_.begin(), start, atmForward, Null<Real>(), Null<Real>(), Null<Real>(), Null<Real>())
            LinearInterpolation(strikes_.begin(), strikes_.end(), volatilities_.begin())
            //NaturalCubicSpline(strikes_.begin(), strikes_.end(), volatilities_.begin())
            );
     }

    Real InterpolatedSmileSection::variance(Real strike) const {
        const Real v = interpolation_->operator()(strike, true);
        return v*v*timeToExpiry_;
    }

    Real InterpolatedSmileSection::volatility(Real strike) const {
        return interpolation_->operator()(strike, true);
    }

    SabrSmileSection::SabrSmileSection(const std::vector<Real>& sabrParameters,
                                       const Time timeToExpiry)
    : timeToExpiry_(timeToExpiry) {

        alpha_ = sabrParameters[0];
        beta_ = sabrParameters[1];
        nu_ = sabrParameters[2];
        rho_ = sabrParameters[3];
        forward_ = sabrParameters[4];

        QL_REQUIRE(forward_>0.0, "forward must be positive: "
                                << io::rate(forward_) << " not allowed");
        QL_REQUIRE(timeToExpiry_>0.0, "expiry time must be positive: "
                                   << timeToExpiry_ << " not allowed");
        QL_REQUIRE(alpha_>0.0, "alpha must be positive: "
                              << alpha_ << " not allowed");
        QL_REQUIRE(beta_>=0.0 && beta_<=1.0, "beta must be in [0.0, 1.0]: "
                                           << beta_ << " not allowed");
        QL_REQUIRE(nu_>=0.0, "nu must be non negative: "
                            << nu_ << " not allowed");
        QL_REQUIRE(rho_*rho_<=1.0, "rho square must be not greater than one: "
                                 << rho_ << " not allowed");

    }


     Real SabrSmileSection::variance(Rate strike) const {
        Volatility vol = unsafeSabrVolatility(strike, forward_, timeToExpiry_,
            alpha_, beta_, nu_, rho_);
        return vol*vol*timeToExpiry_;
     }

     Real SabrSmileSection::volatility(Rate strike) const {
        return unsafeSabrVolatility(strike, forward_, timeToExpiry_,
            alpha_, beta_, nu_, rho_);
     }
}

