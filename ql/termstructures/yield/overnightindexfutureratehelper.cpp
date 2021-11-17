/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2018 Roy Zywina
 Copyright (C) 2019, 2020 Eisuke Tani

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

#include <ql/termstructures/yield/overnightindexfutureratehelper.hpp>
#include <ql/indexes/ibor/sofr.hpp>
#include <ql/utilities/null_deleter.hpp>

namespace QuantLib {

    namespace {

        Date getValidSofrStart(Month month, Year year, Frequency freq) {
            return freq == Monthly ?
                UnitedStates(UnitedStates::GovernmentBond).adjust(Date(1, month, year)) :
                Date::nthWeekday(3, Wednesday, month, year);
        }

        Date getValidSofrEnd(Month month, Year year, Frequency freq) {
            if (freq == Monthly) {
                Calendar dc = UnitedStates(UnitedStates::GovernmentBond);
                Date d = dc.endOfMonth(Date(1, month, year));
                return dc.advance(d, 1*Days);
            } else {
                Date d = getValidSofrStart(month, year, freq) + Period(freq);
                return Date::nthWeekday(3, Wednesday, d.month(), d.year());
            }

        }

    }

    OvernightIndexFutureRateHelper::OvernightIndexFutureRateHelper(
        const Handle<Quote>& price,
        // first day of reference period
        const Date& valueDate,
        // delivery date
        const Date& maturityDate,
        const ext::shared_ptr<OvernightIndex>& overnightIndex,
        const Handle<Quote>& convexityAdjustment,
        RateAveraging::Type averagingMethod)
    : RateHelper(price) {
        ext::shared_ptr<Payoff> payoff;
        ext::shared_ptr<OvernightIndex> index =
            ext::dynamic_pointer_cast<OvernightIndex>(overnightIndex->clone(termStructureHandle_));
        future_ = ext::make_shared<OvernightIndexFuture>(
            index, valueDate, maturityDate, convexityAdjustment, averagingMethod);
        earliestDate_ = valueDate;
        latestDate_ = maturityDate;
    }

    Real OvernightIndexFutureRateHelper::impliedQuote() const {
        future_->recalculate();
        return future_->NPV();
    }

    void OvernightIndexFutureRateHelper::setTermStructure(YieldTermStructure* t) {
        // do not set the relinkable handle as an observer -
        // force recalculation when needed
        bool observer = false;

        ext::shared_ptr<YieldTermStructure> temp(t, null_deleter());
        termStructureHandle_.linkTo(temp, observer);

        RateHelper::setTermStructure(t);
    }

    void OvernightIndexFutureRateHelper::accept(AcyclicVisitor& v) {
        auto* v1 = dynamic_cast<Visitor<OvernightIndexFutureRateHelper>*>(&v);
        if (v1 != nullptr)
            v1->visit(*this);
        else
            RateHelper::accept(v);
    }

    Real OvernightIndexFutureRateHelper::convexityAdjustment() const {
        return future_->convexityAdjustment();
    }


    SofrFutureRateHelper::SofrFutureRateHelper(
        const Handle<Quote>& price,
        Month referenceMonth,
        Year referenceYear,
        Frequency referenceFreq,
        const ext::shared_ptr<OvernightIndex>& overnightIndex,
        const Handle<Quote>& convexityAdjustment,
        RateAveraging::Type averagingMethod)
    : OvernightIndexFutureRateHelper(price,
                                     getValidSofrStart(referenceMonth, referenceYear, referenceFreq),
                                     getValidSofrEnd(referenceMonth, referenceYear, referenceFreq),
                                     overnightIndex,
                                     convexityAdjustment,
                                     averagingMethod) {
        QL_REQUIRE(referenceFreq == Quarterly || referenceFreq == Monthly,
                   "only monthly and quarterly SOFR futures accepted");
        if (referenceFreq == Quarterly) {
            QL_REQUIRE(referenceMonth == Mar || referenceMonth == Jun || referenceMonth == Sep ||
                           referenceMonth == Dec,
                       "quarterly SOFR futures can only start in Mar,Jun,Sep,Dec");
        }
    }

    SofrFutureRateHelper::SofrFutureRateHelper(
        Real price,
        Month referenceMonth,
        Year referenceYear,
        Frequency referenceFreq,
        const ext::shared_ptr<OvernightIndex>& overnightIndex,
        Real convexityAdjustment,
        RateAveraging::Type averagingMethod)
    : OvernightIndexFutureRateHelper(
          Handle<Quote>(ext::make_shared<SimpleQuote>(price)),
          getValidSofrStart(referenceMonth, referenceYear, referenceFreq),
          getValidSofrEnd(referenceMonth, referenceYear, referenceFreq),
          overnightIndex,
          Handle<Quote>(ext::make_shared<SimpleQuote>(convexityAdjustment)),
          averagingMethod) {
        QL_REQUIRE(referenceFreq == Quarterly || referenceFreq == Monthly,
                   "only monthly and quarterly SOFR futures accepted");
        if (referenceFreq == Quarterly) {
            QL_REQUIRE(referenceMonth == Mar || referenceMonth == Jun || referenceMonth == Sep ||
                           referenceMonth == Dec,
                       "quarterly SOFR futures can only start in Mar,Jun,Sep,Dec");
        }
    }

    SofrFutureRateHelper::SofrFutureRateHelper(
        const Handle<Quote>& price,
        Month referenceMonth,
        Year referenceYear,
        Frequency referenceFreq,
        const Handle<Quote>& convexityAdjustment)
    : OvernightIndexFutureRateHelper(price,
            getValidSofrStart(referenceMonth, referenceYear, referenceFreq),
            getValidSofrEnd(referenceMonth, referenceYear, referenceFreq),
            ext::make_shared<Sofr>(),
            convexityAdjustment,
            referenceFreq == Quarterly ? RateAveraging::Compound : RateAveraging::Simple) {
        QL_REQUIRE(referenceFreq == Quarterly || referenceFreq == Monthly,
            "only monthly and quarterly SOFR futures accepted");
        if (referenceFreq == Quarterly) {
            QL_REQUIRE(referenceMonth == Mar || referenceMonth == Jun || referenceMonth == Sep ||
                referenceMonth == Dec,
                "quarterly SOFR futures can only start in Mar,Jun,Sep,Dec");
        }
    }

    SofrFutureRateHelper::SofrFutureRateHelper(
        Real price,
        Month referenceMonth,
        Year referenceYear,
        Frequency referenceFreq,
        Real convexityAdjustment)
    : OvernightIndexFutureRateHelper(
            Handle<Quote>(ext::make_shared<SimpleQuote>(price)),
            getValidSofrStart(referenceMonth, referenceYear, referenceFreq),
            getValidSofrEnd(referenceMonth, referenceYear, referenceFreq),
            ext::make_shared<Sofr>(),
            Handle<Quote>(ext::make_shared<SimpleQuote>(convexityAdjustment)),
            referenceFreq == Quarterly ? RateAveraging::Compound : RateAveraging::Simple) {
        QL_REQUIRE(referenceFreq == Quarterly || referenceFreq == Monthly,
            "only monthly and quarterly SOFR futures accepted");
        if (referenceFreq == Quarterly) {
            QL_REQUIRE(referenceMonth == Mar || referenceMonth == Jun || referenceMonth == Sep ||
                referenceMonth == Dec,
                "quarterly SOFR futures can only start in Mar,Jun,Sep,Dec");
        }
    }
}
