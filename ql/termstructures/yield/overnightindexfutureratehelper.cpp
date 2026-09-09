/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2018 Roy Zywina
 Copyright (C) 2019, 2020 Eisuke Tani
 Copyright (C) 2026 Kyrylo Protsenko

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

#include <ql/termstructures/yield/overnightindexfutureratehelper.hpp>
#include <ql/experimental/termstructures/quotesensitivitycalculator.hpp>
#include <ql/indexes/ibor/sofr.hpp>
#include <ql/utilities/null_deleter.hpp>
namespace QuantLib {

    namespace {

        Date getSofrStart(Month month, Year year, Frequency freq) {
            return freq == Monthly ? Date(1, month, year) :
                   Date::nthWeekday(3, Wednesday, month, year);
        }

        Date getSofrEnd(Month month, Year year, Frequency freq) {
            if (freq == Monthly) {
                return Date::endOfMonth(Date(1, month, year)) + 1;
            } else {
                Date d = getSofrStart(month, year, freq) + Period(freq);
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
        RateAveraging::Type averagingMethod,
        Pillar::Choice pillar,
        const Date& customPillarDate)
    : RateHelper(price) {
        ext::shared_ptr<OvernightIndex> index =
            ext::dynamic_pointer_cast<OvernightIndex>(overnightIndex->clone(termStructureHandle_));
        index->unregisterWith(termStructureHandle_);
        future_ = ext::make_shared<OvernightIndexFuture>(
            index, valueDate, maturityDate, convexityAdjustment, averagingMethod);
        registerWithObservables(future_);
        earliestDate_ = valueDate;
        latestDate_ = maturityDate;
        switch (pillar) {
            case Pillar::MaturityDate:
              pillarDate_ = maturityDate;
              break;
          
            case Pillar::LastRelevantDate:
              pillarDate_ = latestDate_;
              break;
          
            case Pillar::CustomDate:
              QL_REQUIRE(customPillarDate != Date(),
                  "custom pillar date must be provided");
              QL_REQUIRE(customPillarDate >= earliestDate_,
                  "custom pillar date before start of reference period");
              QL_REQUIRE(customPillarDate <= latestDate_,
                  "custom pillar date after end of reference period");
              pillarDate_ = customPillarDate;
              break;
          
            default:
              QL_FAIL("unknown Pillar::Choice");
          }
    }

    Real OvernightIndexFutureRateHelper::impliedQuote() const {
        future_->recalculate();
        return future_->NPV();
    }

    ImpliedQuoteSensitivities OvernightIndexFutureRateHelper::impliedQuoteSensitivitiesByCurve() const {
        if (termStructure_ == nullptr)
            return {};

        const auto& index = future_->overnightIndex();
        const Calendar& calendar = index->fixingCalendar();
        const DayCounter& dc = index->dayCounter();
        Date valueDate = future_->valueDate();
        Date maturityDate = future_->maturityDate();
        Date today = Settings::instance().evaluationDate();
        Time tauRef = dc.yearFraction(valueDate, maturityDate);

        ImpliedQuoteSensitivities result;
        auto& own = result.sensitivities[termStructure_];
        switch (future_->averagingMethod()) {
          case RateAveraging::Compound: {
            // compoundedRate() telescopes to P(start)/P(maturity)
            Date start = valueDate;
            if (today > valueDate) {
                start = calendar.adjust(today);
                if (start < maturityDate && index->hasHistoricalFixing(start))
                    start = std::min(calendar.advance(start, 1, Days), maturityDate);
            }
            if (start >= maturityDate) {
                // fully fixed
                own.emplace_back(maturityDate, 0.0);
                break;
            }

            // NPV = 100*(1-adj-rate), rate = (prod-1)/tauRef
            // prod = C_past * P(start)/P(maturity)
            Real rate = 1.0 - impliedQuote()/100.0 - future_->convexityAdjustment();
            Real prod = rate*tauRef + 1.0;
            Real pastCompounding = prod*termStructure_->discount(maturityDate)
                                   /termStructure_->discount(start);
            detail::addSimpleForwardSensitivities(result, *termStructure_,
                                                  start, maturityDate, tauRef,
                                                  -100.0*pastCompounding);
            break;
          }
          case RateAveraging::Simple: {
            // averagedRate() uses one simple forward per future fixing
            Date d1 = valueDate;
            Date fixingDate = calendar.adjust(d1, Preceding);
            while (d1 < maturityDate) {
                Date d2 = calendar.advance(d1, 1, Days);
                bool forecast = fixingDate > today ||
                    (fixingDate == today && !index->hasHistoricalFixing(fixingDate));
                if (forecast) {
                    // the fixing enters the average with weight w/tauRef
                    Time w = dc.yearFraction(d1, std::min(d2, maturityDate));
                    Time tauFixing = dc.yearFraction(fixingDate, d2);
                    detail::addSimpleForwardSensitivities(result, *termStructure_,
                                                          fixingDate, d2, tauFixing,
                                                          -100.0*w/tauRef);
                }
                fixingDate = d1 = d2;
            }
            if (own.empty())
                // fully fixed
                own.emplace_back(maturityDate, 0.0);
            break;
          }
          default:
            return {};
        }
        result.available = true;
        return result;
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
        const std::variant<Rate, Handle<Quote>>& price,
        Month referenceMonth,
        Year referenceYear,
        Frequency referenceFreq,
        const std::variant<Rate, Handle<Quote>>& convexityAdjustment,
        Pillar::Choice pillar,
        const Date& customPillarDate)
    : OvernightIndexFutureRateHelper(
            handleFromVariant(price),
            getSofrStart(referenceMonth, referenceYear, referenceFreq),
            getSofrEnd(referenceMonth, referenceYear, referenceFreq),
            ext::make_shared<Sofr>(),
            handleFromVariant(convexityAdjustment),
            referenceFreq == Quarterly ? RateAveraging::Compound : RateAveraging::Simple, pillar, customPillarDate) {
        QL_REQUIRE(referenceFreq == Quarterly || referenceFreq == Monthly,
            "only monthly and quarterly SOFR futures accepted");
    }
}
