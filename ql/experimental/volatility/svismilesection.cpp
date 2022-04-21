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

#include <ql/experimental/volatility/sviinterpolation.hpp>
#include <ql/experimental/volatility/svismilesection.hpp>
#include <utility>

namespace QuantLib {

    SviSmileSection::SviSmileSection(Time timeToExpiry, Rate forward, std::vector<Real> sviParams)
    : SmileSection(timeToExpiry, DayCounter()), forward_(forward), params_(std::move(sviParams)) {
        init();
    }

    SviSmileSection::SviSmileSection(const Date& d,
                                     Rate forward,
                                     std::vector<Real> sviParams,
                                     const DayCounter& dc)
    : SmileSection(d, dc, Date()), forward_(forward), params_(std::move(sviParams)) {
        init();
    }

void SviSmileSection::init() {
    QL_REQUIRE(params_.size() == 5,
               "svi expects 5 parameters (a,b,sigma,rho,m) but ("
                   << params_.size() << ") given");
    detail::checkSviParameters(params_[0], params_[1], params_[2], params_[3],
                               params_[4]);
}

Real SviSmileSection::volatilityImpl(Rate strike) const {

    Real k = std::log(std::max(strike, 1E-6) / forward_);
    Real totalVariance = detail::sviTotalVariance(params_[0], params_[1], params_[2],
                                                  params_[3], params_[4],k);
    return std::sqrt(std::max(0.0, totalVariance / exerciseTime()));

}
} // namespace QuantLib
