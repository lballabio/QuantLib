/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2015 Thema Consulting SA

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

#include <ql/experimental/barrieroption/quantodoublebarrieroption.hpp>

namespace QuantLib {

    QuantoDoubleBarrierOption::QuantoDoubleBarrierOption(
                        DoubleBarrier::Type barrierType,
                        Real barrier_lo,
                        Real barrier_hi,
                        Real rebate,
                        const ext::shared_ptr<StrikedTypePayoff>& payoff,
                        const ext::shared_ptr<Exercise>& exercise)
    : DoubleBarrierOption(barrierType, barrier_lo, barrier_hi, rebate, payoff, exercise) {}

    Real QuantoDoubleBarrierOption::qvega() const {
        calculate();
        QL_REQUIRE(qvega_ != Null<Real>(),
                   "exchange rate vega calculation failed");
        return qvega_;
    }

    Real QuantoDoubleBarrierOption::qrho() const {
        calculate();
        QL_REQUIRE(qrho_ != Null<Real>(),
                   "foreign interest rate rho calculation failed");
        return qrho_;
    }

    Real QuantoDoubleBarrierOption::qlambda() const {
        calculate();
        QL_REQUIRE(qlambda_ != Null<Real>(),
                   "quanto correlation sensitivity calculation failed");
        return qlambda_;
    }

    void QuantoDoubleBarrierOption::setupExpired() const {
        DoubleBarrierOption::setupExpired();
        qvega_ = qrho_ = qlambda_ = 0.0;
    }

    void QuantoDoubleBarrierOption::fetchResults(
                                      const PricingEngine::results* r) const {
        DoubleBarrierOption::fetchResults(r);
        const auto* quantoResults = dynamic_cast<const QuantoDoubleBarrierOption::results*>(r);
        QL_ENSURE(quantoResults != nullptr, "no quanto results returned from pricing engine");
        qrho_    = quantoResults->qrho;
        qvega_   = quantoResults->qvega;
        qlambda_ = quantoResults->qlambda;
    }

}

