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
        Date today = Settings::instance().evaluationDate();
        Date npvDate = arguments_.calendar.advance(today, arguments_.settlementDays, Days);
        std::vector<ext::shared_ptr<CashFlow> > cf = arguments_.cashflows;
        Date d1 = effectiveDate();
        for (auto& i : cf) {
            Date d2 = i->date();
            if (d2 > npvDate) {
                d1 = std::max(npvDate, d1);
                Date defaultDate = d1 + (d2 - d1) / 2;

                Real coupon = i->amount() * defaultTS()->survivalProbability(d2);
                Real recovery = notional(defaultDate, arguments_.notionals) * recoveryRate() * (defaultTS()->survivalProbability(d1) -
                                                      defaultTS()->survivalProbability(d2)); 
                NPV_ += coupon * yieldTS()->discount(d2);
                NPV_ += recovery * yieldTS()->discount(defaultDate);
            }
            d1 = d2;
        }

        results_.value = NPV_;
        results_.valuationDate = npvDate;
    }

    Date RiskyBondEngine::effectiveDate() const { return schedule_.dates().front(); }

    Date RiskyBondEngine::maturityDate() const { return schedule_.dates().back(); }

    Real RiskyBondEngine::notional(Date date, std::vector<Real> notionals) const {
        if (date > maturityDate())
            return 0.0;
        Real ntl = notionals.front();
        for (Size i = 0; i < schedule_.size(); i++) {
            if (i < notionals.size() && schedule_[i] <= date)
                ntl = notionals[i];
            else
                break;
        }
        return ntl;
    }

}
