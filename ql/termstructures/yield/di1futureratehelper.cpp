/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2026 Thiago Rodrigues

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

#include <ql/termstructures/yield/di1futureratehelper.hpp>
#include <ql/time/calendars/brazil.hpp>
#include <ql/utilities/null_deleter.hpp>

namespace QuantLib {

    namespace {

        // first trading session of the month on B3
        Date di1Maturity(Month month, Year year) {
            return Brazil(Brazil::Exchange).adjust(Date(1, month, year));
        }

    }

    Di1FutureRateHelper::Di1FutureRateHelper(
        const std::variant<Rate, Handle<Quote>>& rate,
        const Date& maturityDate,
        const ext::shared_ptr<OvernightIndex>& overnightIndex,
        const std::variant<Rate, Handle<Quote>>& convexityAdjustment,
        Pillar::Choice pillar,
        const Date& customPillarDate)
    : RelativeDateRateHelper(rate) {
        QL_REQUIRE(overnightIndex, "null overnight index");
        QL_REQUIRE(maturityDate > evaluationDate_,
                   "maturity date (" << maturityDate << ") must be later than the evaluation date ("
                                     << evaluationDate_ << ")");
        auto index =
            ext::dynamic_pointer_cast<OvernightIndex>(overnightIndex->clone(termStructureHandle_));
        index->unregisterWith(termStructureHandle_);
        future_ = ext::make_shared<Di1Future>(index, maturityDate,
                                              handleFromVariant(convexityAdjustment));
        registerWithObservables(future_);

        maturityDate_ = latestRelevantDate_ = latestDate_ = maturityDate;
        switch (pillar) {
            case Pillar::MaturityDate:
            case Pillar::LastRelevantDate:
                pillarDate_ = latestDate_;
                break;
            case Pillar::CustomDate:
                QL_REQUIRE(customPillarDate != Date(), "custom pillar date must be provided");
                QL_REQUIRE(customPillarDate > evaluationDate_,
                           "custom pillar date not later than the evaluation date");
                QL_REQUIRE(customPillarDate <= latestDate_,
                           "custom pillar date after the maturity date");
                pillarDate_ = customPillarDate;
                break;
            default:
                QL_FAIL("unknown Pillar::Choice");
        }
        Di1FutureRateHelper::initializeDates();
    }

    Di1FutureRateHelper::Di1FutureRateHelper(
        const std::variant<Rate, Handle<Quote>>& rate,
        Month maturityMonth,
        Year maturityYear,
        const ext::shared_ptr<OvernightIndex>& overnightIndex,
        const std::variant<Rate, Handle<Quote>>& convexityAdjustment,
        Pillar::Choice pillar,
        const Date& customPillarDate)
    : Di1FutureRateHelper(rate,
                          di1Maturity(maturityMonth, maturityYear),
                          overnightIndex,
                          convexityAdjustment,
                          pillar,
                          customPillarDate) {}

    void Di1FutureRateHelper::initializeDates() {
        // the accrual period starts on the evaluation date; the other
        // dates don't depend on it and are set in the constructor
        earliestDate_ = evaluationDate_;
    }

    Real Di1FutureRateHelper::impliedQuote() const {
        future_->recalculate();
        return future_->rate();
    }

    ImpliedQuoteSensitivities Di1FutureRateHelper::impliedQuoteSensitivitiesByCurve() const {
        if (termStructure_ == nullptr)
            return {};

        // Q = (P(t0)/P(T))^(1/tau) - 1 + c, with tau = n/252
        Date today = future_->referenceDate();
        Date maturity = future_->maturityDate();
        Time tau = future_->businessDays() / 252.0;
        Real growth = 1.0 + impliedQuote() - future_->convexityAdjustment();

        ImpliedQuoteSensitivities result;
        auto& own = result.sensitivities[termStructure_];
        own.emplace_back(today, growth / (tau * termStructure_->discount(today)));
        own.emplace_back(maturity, -growth / (tau * termStructure_->discount(maturity)));
        result.available = true;
        return result;
    }

    void Di1FutureRateHelper::setTermStructure(YieldTermStructure* t) {
        // do not set the relinkable handle as an observer -
        // force recalculation when needed
        bool observer = false;

        ext::shared_ptr<YieldTermStructure> temp(t, null_deleter());
        termStructureHandle_.linkTo(temp, observer);

        RelativeDateRateHelper::setTermStructure(t);
    }

    void Di1FutureRateHelper::accept(AcyclicVisitor& v) {
        auto* v1 = dynamic_cast<Visitor<Di1FutureRateHelper>*>(&v);
        if (v1 != nullptr)
            v1->visit(*this);
        else
            RateHelper::accept(v);
    }

    Real Di1FutureRateHelper::convexityAdjustment() const {
        return future_->convexityAdjustment();
    }

}
