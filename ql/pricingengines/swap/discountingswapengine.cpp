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
#include <ql/utilities/dataformatters.hpp>

namespace QuantLib {

    DiscountingSwapEngine::DiscountingSwapEngine(
                            const Handle<YieldTermStructure>& discountCurve,
                            boost::optional<bool> includeSettlementDateFlows,
                            Date settlementDate,
                            Date npvDate)
    : discountCurve_(discountCurve),
      includeSettlementDateFlows_(includeSettlementDateFlows),
      settlementDate_(settlementDate), npvDate_(npvDate) {
        registerWith(discountCurve_);
    }

    void DiscountingSwapEngine::calculate() const {
        QL_REQUIRE(!discountCurve_.empty(),
                   "discounting term structure handle is empty");

        results_.value = 0.0;
        results_.errorEstimate = Null<Real>();

        Date refDate = (*discountCurve_)->referenceDate();

        Date settlementDate = settlementDate_;
        if (settlementDate_==Date()) {
            settlementDate = refDate;
        } else {
            QL_REQUIRE(settlementDate>=refDate,
                       "settlement date (" << settlementDate << ") before "
                       "discount curve reference date (" << refDate << ")");
        }

        results_.valuationDate = npvDate_;
        if (npvDate_==Date()) {
            results_.valuationDate = refDate;
        } else {
            QL_REQUIRE(npvDate_>=refDate,
                       "npv date (" << npvDate_  << ") before "
                       "discount curve reference date (" << refDate << ")");
        }
        results_.additionalResults["npvDateDiscount"] =
                            discountCurve_->discount(results_.valuationDate);

        results_.legNPV.resize(arguments_.legs.size());
        results_.legBPS.resize(arguments_.legs.size());
        std::vector<DiscountFactor> startDiscounts(arguments_.legs.size());

        bool includeRefDateFlows =
            includeSettlementDateFlows_ ?
            *includeSettlementDateFlows_ :
            Settings::instance().includeReferenceDateCashFlows();

        for (Size i=0; i<arguments_.legs.size(); ++i) {
            try {
                results_.legNPV[i] = arguments_.payer[i] *
                    CashFlows::npv(arguments_.legs[i],
                                   **discountCurve_,
                                   includeRefDateFlows,
                                   settlementDate,
                                   results_.valuationDate);
                results_.legBPS[i] = arguments_.payer[i] *
                    CashFlows::bps(arguments_.legs[i],
                                   **discountCurve_,
                                   includeRefDateFlows,
                                   settlementDate,
                                   results_.valuationDate);
            } catch (std::exception &e) {
                QL_FAIL(io::ordinal(i+1) << " leg: " << e.what());
            }
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
