/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2023 Marcin Rybacki

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
        calculate();
        return amount_;
    }

    void EquityCashFlow::performCalculations() const {
        if (!pricer_) {
            IndexedCashFlow::performCalculations();
            return;
        }
        pricer_->initialize(*this);
        amount_ = notional() * pricer_->price();
    }

    Real EquityCashFlow::indexLevel(const Date& d) const {
        auto equityIndex = ext::dynamic_pointer_cast<EquityIndex>(index());
        QL_REQUIRE(equityIndex, "equity index required");
        if (pricer_)
            return pricer_->indexLevel(equityIndex, d);
        return equityIndex->fixing(d);
    }

    Real EquityCashFlow::indexUnits() const {
        return notional() / indexLevel(baseDate());
    }

    EquityNotionalResetCashFlow::EquityNotionalResetCashFlow(Real initialNotional,
                                                             ext::shared_ptr<EquityIndex> index,
                                                             const Date& initialDate,
                                                             const Date& baseDate,
                                                             const Date& fixingDate,
                                                             const Date& paymentDate)
    : EquityCashFlow(initialNotional, std::move(index), baseDate, fixingDate, paymentDate),
      initialDate_(initialDate) {
        QL_REQUIRE(initialDate_ <= baseDate,
                   "initial date (" << initialDate_ << ") cannot be later than base date ("
                                    << baseDate << ")");
    }

    Real EquityNotionalResetCashFlow::notional() const {
        return initialNotional() * indexLevel(baseDate()) / indexLevel(initialDate_);
    }

    EquityDividendCashFlow::EquityDividendCashFlow(ext::shared_ptr<Dividend> dividend,
                                                   ext::shared_ptr<EquityCashFlow> equityCashFlow,
                                                   Real dividendFactor,
                                                   const Date& paymentDate)
    : dividend_(std::move(dividend)), equityCashFlow_(std::move(equityCashFlow)),
      dividendFactor_(dividendFactor), paymentDate_(paymentDate) {
        QL_REQUIRE(dividend_, "no dividend provided");
        QL_REQUIRE(equityCashFlow_, "no equity cash flow provided");
        QL_REQUIRE(dividendFactor_ >= 0.0,
                   "dividend factor (" << dividendFactor_ << ") cannot be negative");
        const Date exDate = dividend_->date();
        QL_REQUIRE(exDate > equityCashFlow_->baseDate() &&
                       exDate <= equityCashFlow_->fixingDate(),
                   "ex-dividend date (" << exDate << ") outside the equity period ("
                                        << equityCashFlow_->baseDate() << ", "
                                        << equityCashFlow_->fixingDate() << "]");
        // the amount is not cached, so notifications can always be forwarded
        alwaysForwardNotifications();
        registerWith(dividend_);
        registerWith(equityCashFlow_);
        registerWith(equityCashFlow_->index());
    }

    Real EquityDividendCashFlow::amount() const {
        return dividendFactor_ * dividend_->amount() * equityCashFlow_->indexUnits();
    }

    Real EquityCashFlowPricer::indexLevel(const ext::shared_ptr<EquityIndex>& index,
                                          const Date& d) const {
        QL_REQUIRE(index, "equity index required");
        return index->fixing(d);
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
        QL_REQUIRE(index_->returnType() == EquityIndex::Price,
                   "Quanto pricer only supports price return equity indexes.");
        baseDate_ = cashFlow.baseDate();
        fixingDate_ = cashFlow.fixingDate();
        QL_REQUIRE(fixingDate_ >= baseDate_, "Fixing date cannot fall before base date.");
        growthOnlyPayoff_ = cashFlow.growthOnly();

        checkInputs();
    }

    void EquityQuantoCashFlowPricer::checkInputs() const {
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

    ext::shared_ptr<EquityIndex>
    EquityQuantoCashFlowPricer::quantoIndex(const ext::shared_ptr<EquityIndex>& index,
                                            Real strike) const {
        Handle<YieldTermStructure> dividendHandle =
            configureDividendHandle(index->equityDividendCurve());

        Handle<YieldTermStructure> quantoTermStructure(ext::make_shared<QuantoTermStructure>(
            dividendHandle, quantoCurrencyTermStructure_,
            index->equityInterestRateCurve(), equityVolatility_, strike, fxVolatility_, 1.0,
            correlation_->value()));
        return index->clone(quantoCurrencyTermStructure_, quantoTermStructure, index->spot());
    }

    Real EquityQuantoCashFlowPricer::price() const {
        Real strike = index_->fixing(fixingDate_);
        ext::shared_ptr<EquityIndex> adjustedIndex = quantoIndex(index_, strike);

        Real I0 = adjustedIndex->fixing(baseDate_);
        Real I1 = adjustedIndex->fixing(fixingDate_);

        if (growthOnlyPayoff_)
            return I1 / I0 - 1.0;
        return I1 / I0;
    }

    Real EquityQuantoCashFlowPricer::indexLevel(const ext::shared_ptr<EquityIndex>& index,
                                                const Date& d) const {
        QL_REQUIRE(index, "Equity index required.");
        QL_REQUIRE(index->returnType() == EquityIndex::Price,
                   "Quanto pricer only supports price return equity indexes.");
        checkInputs();
        Real strike = index->fixing(d);
        return quantoIndex(index, strike)->fixing(d);
    }
}
