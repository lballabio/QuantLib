/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2023 Marcin Rybacki

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

#include <ql/cashflows/equityquantocashflow.hpp>
#include <ql/cashflows/indexedcashflow.hpp>
#include <ql/indexes/equityindex.hpp>
#include <ql/termstructures/yield/quantotermstructure.hpp>

namespace QuantLib {

    void setCouponPricer(const Leg& leg, const ext::shared_ptr<EquityQuantoCashFlowPricer>& p) {
        for (const auto& i : leg) {
            ext::shared_ptr<EquityQuantoCashFlow> c =
                ext::dynamic_pointer_cast<EquityQuantoCashFlow>(i);
            if (c != nullptr)
                c->setPricer(p);
        }
    }

    EquityQuantoCashFlow::EquityQuantoCashFlow(Real notional,
        ext::shared_ptr<EquityIndex> equityIndex,
        const Date& startDate,
        const Date& endDate,
        const Date& paymentDate)
     : notional_(notional), equityIndex_(std::move(equityIndex)), startDate_(startDate),
       endDate_(endDate), paymentDate_(paymentDate) {
        registerWith(equityIndex_);
        registerWith(Settings::instance().evaluationDate());
    }

    void EquityQuantoCashFlow::setPricer(const ext::shared_ptr<EquityQuantoCashFlowPricer>& pricer) {
        if (pricer_ != nullptr)
            unregisterWith(pricer_);
        pricer_ = pricer;
        if (pricer_ != nullptr)
            registerWith(pricer_);
        update();
    }

    Real EquityQuantoCashFlow::amount() const {
        QL_REQUIRE(pricer_, "Equity quanto cash flow pricer not set.");
        pricer_->initialize(*this);
        return pricer_->quantoAmount();
    }

	EquityQuantoCashFlowPricer::EquityQuantoCashFlowPricer(
        Handle<YieldTermStructure> quantoCurrencyTermStructure,
        Handle<BlackVolTermStructure> equityVolatility,
        Handle<BlackVolTermStructure> fxVolatility,
        Handle<Quote> correlation)
    : quantoCurrencyTermStructure_(std::move(quantoCurrencyTermStructure)),
      equityVolatility_(std::move(equityVolatility)), fxVolatility_(std::move(fxVolatility)),
      correlation_(std::move(correlation)){
        registerWith(quantoCurrencyTermStructure_);
        registerWith(equityVolatility_);
        registerWith(fxVolatility_);
        registerWith(correlation_);
    }

    void EquityQuantoCashFlowPricer::initialize(const EquityQuantoCashFlow& cashFlow) {
        cashFlow_ = dynamic_cast<const EquityQuantoCashFlow*>(&cashFlow);
        QL_REQUIRE(cashFlow_, "Equity quanto cash flow needed.");
        QL_REQUIRE(
            !quantoCurrencyTermStructure_.empty() && !equityVolatility_.empty() &&
                !fxVolatility_.empty(),
            "Quanto currency, equity and FX volatility term structure handles cannot be empty.");
        QL_REQUIRE(quantoCurrencyTermStructure_->referenceDate() ==
                           equityVolatility_->referenceDate() &&
                       equityVolatility_->referenceDate() == fxVolatility_->referenceDate(),
                   "Quanto currency term structure, equity and FX volatility need to have the same "
                   "reference date.");
    }

    Real EquityQuantoCashFlowPricer::quantoAmount() const {
        const ext::shared_ptr<EquityIndex>& originalIndex = cashFlow_->equityIndex();
        auto endDate = cashFlow_->endDate();
        auto strike = originalIndex->fixing(endDate);
        
        Handle<YieldTermStructure> quantoTermStructure(ext::make_shared<QuantoTermStructure>(
            originalIndex->equityDividendCurve(), quantoCurrencyTermStructure_,
            originalIndex->equityInterestRateCurve(), equityVolatility_, strike, fxVolatility_, 1.0,
            correlation_->value()));
        
        ext::shared_ptr<EquityIndex> quantoIndex = originalIndex->clone(
            quantoCurrencyTermStructure_, quantoTermStructure, originalIndex->spot());

        Real I0 = quantoIndex->fixing(cashFlow_->startDate());
        Real I1 = quantoIndex->fixing(endDate);

        return cashFlow_->notional() * (I1 / I0 - 1.0);
    }
}