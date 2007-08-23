/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Giorgio Facchinetti

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

#include <ql/pricingengines/discountengine.hpp>


namespace QuantLib {

    DiscountEngine::DiscountEngine(
                              const Handle<YieldTermStructure>& discountCurve)
    : discountCurve_(discountCurve) {
        registerWith(discountCurve_);
    }

    void DiscountEngine::update() {
        notifyObservers();
    }

    Handle<YieldTermStructure> DiscountEngine::discountCurve() const {
        return discountCurve_;
    }

    Real DiscountEngine::npv(const Leg& cashflows,
                             const Date& settlementDate,
                             const Date& npvDate,
                             Integer exDividendDays) {

        return CashFlows::npv(cashflows,
                              **discountCurve_,
                              settlementDate,
                              npvDate,
                              exDividendDays);
    }

    Real DiscountEngine::bps(const Leg& cashflows,
                             const Date& settlementDate,
                             const Date& npvDate,
                             Integer exDividendDays) {

        return CashFlows::bps(cashflows,
                              **discountCurve_,
                              settlementDate,
                              npvDate,
                              exDividendDays);
    }

}
