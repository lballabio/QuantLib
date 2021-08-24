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
#include <ql/cashflows/coupon.hpp>


namespace QuantLib {

    RiskyBondEngine::RiskyBondEngine(Schedule schedule,
                                     Handle<DefaultProbabilityTermStructure> defaultTS,
                                     Real recoveryRate,
                                     Handle<YieldTermStructure> yieldTS)
    : schedule_(schedule),
      defaultTS_(std::move(defaultTS)), 
      recoveryRate_(recoveryRate), 
      yieldTS_(std::move(yieldTS))
      {}

    void RiskyBondEngine::calculate() const {
        Real NPV_ = 0;
        Date npvDate = arguments_.settlementDate;
        std::vector<ext::shared_ptr<CashFlow> > cf = arguments_.cashflows;
        Date d1 = CashFlows::startDate(arguments_.cashflows);
        for (auto& i : cf) {
            Date d2 = i->date();
            if (d2 > npvDate) {
                d1 = std::max(npvDate, d1);
                Date defaultDate = d1 + (d2 - d1) / 2;

                Real couponAmount = i->amount() * defaultTS()->survivalProbability(d2);

                auto coupon = ext::dynamic_pointer_cast<Coupon>(i);
                if (coupon != nullptr) {
                    Real recovery = coupon->nominal() * recoveryRate() *
                                    (defaultTS()->survivalProbability(d1) -
                                     defaultTS()->survivalProbability(d2)); 
                    NPV_ += recovery * yieldTS()->discount(defaultDate);
                }
                
                NPV_ += couponAmount * yieldTS()->discount(d2);
            }
            d1 = d2;
        }

        results_.value = NPV_;
        results_.valuationDate = npvDate;
    }

}
