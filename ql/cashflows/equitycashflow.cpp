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

#include <ql/cashflows/equitycashflow.hpp>
#include <ql/cashflows/indexedcashflow.hpp>
#include <ql/indexes/equityindex.hpp>
#include <ql/termstructures/yield/quantotermstructure.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/time/calendars/nullcalendar.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>

namespace QuantLib {

    namespace {
        Handle<YieldTermStructure>
            configureDividendHandle(const Handle<YieldTermStructure>& dividendHandle) {
            if (dividendHandle.empty()) {
                ext::shared_ptr<YieldTermStructure> flatTs(ext::make_shared<FlatForward>(
                    0, NullCalendar(), Handle<Quote>(ext::make_shared<SimpleQuote>(0.0)),
                    Actual365Fixed()));
                return Handle<YieldTermStructure>(flatTs);
            }
            return dividendHandle;
        }
    }

    void setCouponPricer(const Leg& leg, const ext::shared_ptr<EquityCashFlowPricer>& p) {
        for (const auto& i : leg) {
            ext::shared_ptr<EquityCashFlow> c =
                ext::dynamic_pointer_cast<EquityCashFlow>(i);
            if (c != nullptr)
                c->setPricer(p);
        }
    }

    EquityCashFlow::EquityCashFlow(Real notional,
                                   ext::shared_ptr<EquityIndex> index,
                                   const Date& baseDate,
                                   const Date& fixingDate,
                                   const Date& paymentDate,
                                   bool growthOnly)
    : IndexedCashFlow(notional, std::move(index), baseDate, fixingDate, paymentDate, growthOnly) {}

    void EquityCashFlow::setPricer(const ext::shared_ptr<EquityCashFlowPricer>& pricer) {
        if (pricer_ != nullptr)
            unregisterWith(pricer_);
        pricer_ = pricer;
        if (pricer_ != nullptr)
            registerWith(pricer_);
        update();
    }

    Real EquityCashFlow::amount() const {
        if (!pricer_)
            return IndexedCashFlow::amount();
        pricer_->initialize(*this);
        return notional() * pricer_->price();
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

    void EquityQuantoCashFlowPricer::initialize(const EquityCashFlow& cashFlow) {
        index_ = ext::dynamic_pointer_cast<EquityIndex>(cashFlow.index());
        if (!index_) {
            QL_FAIL("Equity index required.");
        }
        baseDate_ = cashFlow.baseDate();
        fixingDate_ = cashFlow.fixingDate();
        QL_REQUIRE(fixingDate_ >= baseDate_, "Fixing date cannot fall before base date.");
        growthOnlyPayoff_ = cashFlow.growthOnly();
        
        QL_REQUIRE(!quantoCurrencyTermStructure_.empty(),
                   "Quanto currency term structure handle cannot be empty.");
        QL_REQUIRE(!equityVolatility_.empty(),
                   "Equity volatility term structure handle cannot be empty.");
        QL_REQUIRE(!fxVolatility_.empty(),
                   "FX volatility term structure handle cannot be empty.");
        QL_REQUIRE(!correlation_.empty(), "Correlation handle cannot be empty.");

        QL_REQUIRE(quantoCurrencyTermStructure_->referenceDate() ==
                           equityVolatility_->referenceDate() &&
                       equityVolatility_->referenceDate() == fxVolatility_->referenceDate(),
                   "Quanto currency term structure, equity and FX volatility need to have the same "
                   "reference date.");
    }

    Real EquityQuantoCashFlowPricer::price() const {
        Real strike = index_->fixing(fixingDate_);
        Handle<YieldTermStructure> dividendHandle =
            configureDividendHandle(index_->equityDividendCurve());

        Handle<YieldTermStructure> quantoTermStructure(ext::make_shared<QuantoTermStructure>(
            dividendHandle, quantoCurrencyTermStructure_,
            index_->equityInterestRateCurve(), equityVolatility_, strike, fxVolatility_, 1.0,
            correlation_->value()));
        ext::shared_ptr<EquityIndex> quantoIndex =
            index_->clone(quantoCurrencyTermStructure_, quantoTermStructure, index_->spot());

        Real I0 = quantoIndex->fixing(baseDate_);
        Real I1 = quantoIndex->fixing(fixingDate_);

        if (growthOnlyPayoff_)
            return I1 / I0 - 1.0;
        return I1 / I0;
    }
}