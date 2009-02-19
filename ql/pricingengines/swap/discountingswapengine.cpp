/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007, 2009 StatPro Italia srl

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

#include <ql/pricingengines/swap/discountingswapengine.hpp>
#include <ql/cashflows/cashflows.hpp>

namespace QuantLib {

    DiscountingSwapEngine::DiscountingSwapEngine(
                              const Handle<YieldTermStructure>& discountCurve)
    : discountCurve_(discountCurve) {
        registerWith(discountCurve_);
    }

    void DiscountingSwapEngine::calculate() const {
        QL_REQUIRE(!discountCurve_.empty(),
                   "no discounting term structure set");

        results_.value = 0.0;
        results_.errorEstimate = Null<Real>();
        results_.legNPV.resize(arguments_.legs.size());
        results_.legBPS.resize(arguments_.legs.size());
        std::vector<DiscountFactor> startDiscounts(arguments_.legs.size());
        for (Size i=0; i<arguments_.legs.size(); ++i) {
            results_.legNPV[i] =
                arguments_.payer[i] * CashFlows::npv(arguments_.legs[i],
                                                     **discountCurve_);
            results_.legBPS[i] =
                arguments_.payer[i] * CashFlows::bps(arguments_.legs[i],
                                                     **discountCurve_);
            results_.value += results_.legNPV[i];
            try {
                Date d = CashFlows::startDate(arguments_.legs[i]);
                startDiscounts[i] = discountCurve_->discount(d);
            } catch (...) {
                startDiscounts[i] = Null<DiscountFactor>();
            }
        }
        results_.additionalResults["startDiscounts"] = startDiscounts;
    }

}

