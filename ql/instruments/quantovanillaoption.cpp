/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2002, 2003 Ferdinando Ametrano
 Copyright (C) 2007 StatPro Italia srl

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

#include <ql/instruments/quantovanillaoption.hpp>

namespace QuantLib {

    QuantoVanillaOption::QuantoVanillaOption(
                   const std::shared_ptr<StrikedTypePayoff>& payoff,
                   const std::shared_ptr<Exercise>& exercise)
    : OneAssetOption(payoff, exercise) {}

    Real QuantoVanillaOption::qvega() const {
        calculate();
        QL_REQUIRE(qvega_ != Null<Real>(),
                   "exchange rate vega calculation failed");
        return qvega_;
    }

    Real QuantoVanillaOption::qrho() const {
        calculate();
        QL_REQUIRE(qrho_ != Null<Real>(),
                   "foreign interest rate rho calculation failed");
        return qrho_;
    }

    Real QuantoVanillaOption::qlambda() const {
        calculate();
        QL_REQUIRE(qlambda_ != Null<Real>(),
                   "quanto correlation sensitivity calculation failed");
        return qlambda_;
    }

    void QuantoVanillaOption::setupExpired() const {
        OneAssetOption::setupExpired();
        qvega_ = qrho_ = qlambda_ = 0.0;
    }

    void QuantoVanillaOption::fetchResults(
                                      const PricingEngine::results* r) const {
        OneAssetOption::fetchResults(r);
        const auto* quantoResults = dynamic_cast<const QuantoVanillaOption::results*>(r);
        QL_ENSURE(quantoResults != nullptr, "no quanto results returned from pricing engine");
        qrho_    = quantoResults->qrho;
        qvega_   = quantoResults->qvega;
        qlambda_ = quantoResults->qlambda;
    }

}

