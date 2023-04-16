/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Paul Farrington

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

#include <ql/instruments/quantobarrieroption.hpp>

namespace QuantLib {

    QuantoBarrierOption::QuantoBarrierOption(
                        Barrier::Type barrierType,
                        Real barrier,
                        Real rebate,
                        const std::shared_ptr<StrikedTypePayoff>& payoff,
                        const std::shared_ptr<Exercise>& exercise)
    : BarrierOption(barrierType, barrier, rebate, payoff, exercise) {}

    Real QuantoBarrierOption::qvega() const {
        calculate();
        QL_REQUIRE(qvega_ != Null<Real>(),
                   "exchange rate vega calculation failed");
        return qvega_;
    }

    Real QuantoBarrierOption::qrho() const {
        calculate();
        QL_REQUIRE(qrho_ != Null<Real>(),
                   "foreign interest rate rho calculation failed");
        return qrho_;
    }

    Real QuantoBarrierOption::qlambda() const {
        calculate();
        QL_REQUIRE(qlambda_ != Null<Real>(),
                   "quanto correlation sensitivity calculation failed");
        return qlambda_;
    }

    void QuantoBarrierOption::setupExpired() const {
        BarrierOption::setupExpired();
        qvega_ = qrho_ = qlambda_ = 0.0;
    }

    void QuantoBarrierOption::fetchResults(
                                      const PricingEngine::results* r) const {
        BarrierOption::fetchResults(r);
        const auto* quantoResults = dynamic_cast<const QuantoBarrierOption::results*>(r);
        QL_ENSURE(quantoResults != nullptr, "no quanto results returned from pricing engine");
        qrho_    = quantoResults->qrho;
        qvega_   = quantoResults->qvega;
        qlambda_ = quantoResults->qlambda;
    }

}

