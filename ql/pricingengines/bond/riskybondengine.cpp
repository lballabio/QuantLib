/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2021 Lew Wei Hao

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

#include <ql/cashflows/cashflows.hpp>
#include <utility>
#include <ql/experimental/credit/riskybond.hpp>
#include <ql/pricingengines/bond/riskybondengine.hpp>


namespace QuantLib {

    void RiskyBondEngine::calculate() const {
        Real NPV_ = 0;
        Date today = Settings::instance().evaluationDate();
        Date npvDate = arguments_.bond->calendar().advance(today, arguments_.bond->settlementDays(), Days);
        std::vector<ext::shared_ptr<CashFlow> > cf = arguments_.bond->cashflows();
        Date d1 = arguments_.bond->effectiveDate();
        for (auto& i : cf) {
            Date d2 = i->date();
            if (d2 > npvDate) {
                d1 = std::max(npvDate, d1);
                Date defaultDate = d1 + (d2 - d1) / 2;

                Real coupon = i->amount() * arguments_.bond->defaultTS()->survivalProbability(d2);
                Real recovery = arguments_.bond->notional(defaultDate) *arguments_.bond->recoveryRate() * (arguments_.bond->defaultTS()->survivalProbability(d1) -
                                                      arguments_.bond->defaultTS()->survivalProbability(d2));
                NPV_ += coupon * arguments_.bond->yieldTS()->discount(d2);
                NPV_ += recovery * arguments_.bond->yieldTS()->discount(defaultDate);
            }
            d1 = d2;
        }

        results_.value = NPV_;
        results_.valuationDate = npvDate;
    }

}
