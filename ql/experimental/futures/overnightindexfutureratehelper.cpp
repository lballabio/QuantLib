/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2018 Roy Zywina

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

#include <ql/experimental/futures/overnightindexfutureratehelper.hpp>
#include <ql/utilities/null_deleter.hpp>

namespace QuantLib {

    namespace {

        Date getValidSofrStart(Month month, Year year) {
            return Date::nthWeekday(3,Wednesday,month,year);
        }

        Date getValidSofrEnd(Month month, Year year,Frequency freq) {
            Date d = getValidSofrStart(month,year) + Period(freq);
            return Date::nthWeekday(3,Wednesday,d.month(),d.year());
        }

    }

    OvernightIndexFutureRateHelper::OvernightIndexFutureRateHelper(
        const Handle<Quote>& price,
        // first day of reference period
        const Date& valueDate,
        // delivery date
        const Date& maturityDate,
        const ext::shared_ptr<OvernightIndex>& overnightIndex,
        const Handle<Quote>& convexityAdjustment)
      :RateHelper(price)
    {
        ext::shared_ptr<Payoff> payoff;
        future_ = ext::make_shared<OvernightIndexFuture>(overnightIndex,
            payoff, valueDate, maturityDate, termStructureHandle_,
            convexityAdjustment);
        earliestDate_ = valueDate;
        latestDate_ = maturityDate;
    }

    Real OvernightIndexFutureRateHelper::impliedQuote() const{
        return future_->spotValue();
    }

    void OvernightIndexFutureRateHelper::setTermStructure(YieldTermStructure *t){
        // do not set the relinkable handle as an observer -
        // force recalculation when needed
        bool observer = false;

        ext::shared_ptr<YieldTermStructure> temp(t, null_deleter());
        termStructureHandle_.linkTo(temp, observer);

        RateHelper::setTermStructure(t);
    }

    void OvernightIndexFutureRateHelper::accept(AcyclicVisitor &v){
        Visitor<OvernightIndexFutureRateHelper>* v1 =
            dynamic_cast<Visitor<OvernightIndexFutureRateHelper>*>(&v);
        if (v1 != 0)
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
        const Handle<Quote>& convexityAdjustment)
      :OvernightIndexFutureRateHelper(price,
        getValidSofrStart(referenceMonth,referenceYear),
        getValidSofrEnd(referenceMonth,referenceYear,referenceFreq),
        overnightIndex,convexityAdjustment)
    {
        QL_REQUIRE(referenceFreq==Quarterly || referenceFreq==Monthly,
            "only monthly and quarterly SOFR futures accepted");
        if (referenceFreq==Quarterly) {
            QL_REQUIRE(referenceMonth==Mar || referenceMonth==Jun ||
                referenceMonth==Sep || referenceMonth==Dec,
                "quarterly SOFR futures can only start in Mar,Jun,Sep,Dec");
        }
    }

    SofrFutureRateHelper::SofrFutureRateHelper(
        Real price,
        Month referenceMonth,
        Year referenceYear,
        Frequency referenceFreq,
        const ext::shared_ptr<OvernightIndex>& overnightIndex,
        Real convexityAdjustment)
      :OvernightIndexFutureRateHelper(
        Handle<Quote>(ext::make_shared<SimpleQuote>(price)),
        getValidSofrStart(referenceMonth,referenceYear),
        getValidSofrEnd(referenceMonth,referenceYear,referenceFreq),
        overnightIndex,
        Handle<Quote>(ext::make_shared<SimpleQuote>(convexityAdjustment)))
    {
        QL_REQUIRE(referenceFreq==Quarterly || referenceFreq==Monthly,
            "only monthly and quarterly SOFR futures accepted");
        if (referenceFreq==Quarterly) {
            QL_REQUIRE(referenceMonth==Mar || referenceMonth==Jun ||
                referenceMonth==Sep || referenceMonth==Dec,
                "quarterly SOFR futures can only start in Mar,Jun,Sep,Dec");
        }
    }

}
