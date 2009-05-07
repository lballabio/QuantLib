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

#include <ql/pricingengines/bond/discountingbondengine.hpp>
#include <ql/cashflows/cashflows.hpp>

namespace QuantLib {

    DiscountingBondEngine::DiscountingBondEngine(
                              const Handle<YieldTermStructure>& discountCurve)
    : discountCurve_(discountCurve) {
        registerWith(discountCurve_);
    }

    void DiscountingBondEngine::calculate() const {
        QL_REQUIRE(!discountCurve_.empty(),
                   "discounting term structure handle is empty");

        results_.valuationDate = (*discountCurve_)->referenceDate();
        results_.value = CashFlows::npv(arguments_.cashflows,
                                        **discountCurve_,
                                        true, results_.valuationDate);

        const Date& settlementDate = arguments_.settlementDate;
        results_.settlementValue = CashFlows::npv(arguments_.cashflows,
                                                  **discountCurve_,
                                                  false, settlementDate);
    }

}
