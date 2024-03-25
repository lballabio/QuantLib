/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009, 2012 Roland Lichters
 Copyright (C) 2009, 2012 Ferdinando Ametrano

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

#include <ql/instruments/makeois.hpp>
#include <ql/instruments/simplifynotificationgraph.hpp>
#include <ql/pricingengines/swap/discountingswapengine.hpp>
#include <ql/termstructures/yield/oisratehelper.hpp>
#include <ql/utilities/null_deleter.hpp>
#include <utility>

namespace QuantLib {
    namespace detail {
        OISRateHelperBase::OISRateHelperBase(const Handle<Quote>& fixedRate,
                                             const ext::shared_ptr<OvernightIndex>& overnightIndex,
                                             Handle<YieldTermStructure> discountingCurve,
                                             Pillar::Choice pillar,
                                             Date customPillarDate)
        : SwapRateHelperBase(fixedRate, overnightIndex, std::move(discountingCurve), pillar,
            customPillarDate) {}

        Real OISRateHelperBase::impliedQuote() const {
            QL_REQUIRE(termStructure_ != nullptr, "term structure not set");
            // we didn't register as observers - force calculation
            swap_->deepUpdate();
            return swap_->fairRate();
        }
    }

    OISRateHelper::OISRateHelper(Natural settlementDays,
                                 const Period& tenor, // swap maturity
                                 const Handle<Quote>& fixedRate,
                                 const ext::shared_ptr<OvernightIndex>& overnightIndex,
                                 Handle<YieldTermStructure> discount,
                                 bool telescopicValueDates,
                                 Integer paymentLag,
                                 BusinessDayConvention paymentConvention,
                                 Frequency paymentFrequency,
                                 Calendar paymentCalendar,
                                 const Period& forwardStart,
                                 const Spread overnightSpread,
                                 Pillar::Choice pillar,
                                 Date customPillarDate,
                                 RateAveraging::Type averagingMethod,
                                 ext::optional<bool> endOfMonth,
                                 ext::optional<Frequency> fixedPaymentFrequency,
                                 Calendar fixedCalendar)
    : base_type(fixedRate, overnightIndex, std::move(discount), pillar, customPillarDate),
      settlementDays_(settlementDays), tenor_(tenor), telescopicValueDates_(telescopicValueDates),
      paymentLag_(paymentLag), paymentConvention_(paymentConvention),
      paymentFrequency_(paymentFrequency), paymentCalendar_(std::move(paymentCalendar)),
      forwardStart_(forwardStart), overnightSpread_(overnightSpread),
      averagingMethod_(averagingMethod), endOfMonth_(endOfMonth),
      fixedPaymentFrequency_(fixedPaymentFrequency), fixedCalendar_(std::move(fixedCalendar)) {
        OISRateHelper::initializeDates();
    }

    void OISRateHelper::initializeDates() {
        // input discount curve Handle might be empty now but it could
        //    be assigned a curve later; use a RelinkableHandle here
        auto tmp = MakeOIS(tenor_, overnightIndex(), 0.0, forwardStart_)
            .withDiscountingTermStructure(discountRelinkableHandle_)
            .withSettlementDays(settlementDays_)
            .withTelescopicValueDates(telescopicValueDates_)
            .withPaymentLag(paymentLag_)
            .withPaymentAdjustment(paymentConvention_)
            .withPaymentFrequency(paymentFrequency_)
            .withPaymentCalendar(paymentCalendar_)
            .withOvernightLegSpread(overnightSpread_)
            .withAveragingMethod(averagingMethod_);
        if (endOfMonth_) {
            tmp.withEndOfMonth(*endOfMonth_);
        }
        if (fixedPaymentFrequency_) {
            tmp.withFixedLegPaymentFrequency(*fixedPaymentFrequency_);
        }
        if (!fixedCalendar_.empty()) {
            tmp.withFixedLegCalendar(fixedCalendar_);
        }
        setInstrument<OvernightIndexedSwap>(tmp);
    }

    void OISRateHelper::accept(AcyclicVisitor& v) {
        auto* v1 = dynamic_cast<Visitor<OISRateHelper>*>(&v);
        if (v1 != nullptr)
            v1->visit(*this);
        else
            base_type::accept(v);
    }

    DatedOISRateHelper::DatedOISRateHelper(const Date& startDate,
                                           const Date& endDate,
                                           const Handle<Quote>& fixedRate,
                                           const ext::shared_ptr<OvernightIndex>& index,
                                           Handle<YieldTermStructure> discount,
                                           bool telescopicValueDates,
                                           RateAveraging::Type averagingMethod,
                                           Integer paymentLag,
                                           BusinessDayConvention paymentConvention,
                                           Frequency paymentFrequency,
                                           const Calendar& paymentCalendar,
                                           const Period& forwardStart,
                                           Spread overnightSpread,
                                           ext::optional<bool> endOfMonth,
                                           ext::optional<Frequency> fixedPaymentFrequency,
                                           const Calendar& fixedCalendar,
                                           Pillar::Choice pillar,
                                           Date customPillarDate)
    : base_type(fixedRate, index, std::move(discount), pillar, customPillarDate) {
        // input discount curve Handle might be empty now but it could
        //    be assigned a curve later; use a RelinkableHandle here
        auto tmp = MakeOIS(Period(), overnightIndex(), 0.0, forwardStart)
            .withDiscountingTermStructure(discountRelinkableHandle_)
            .withEffectiveDate(startDate)
            .withTerminationDate(endDate)
            .withTelescopicValueDates(telescopicValueDates)
            .withPaymentLag(paymentLag)
            .withPaymentAdjustment(paymentConvention)
            .withPaymentFrequency(paymentFrequency)
            .withPaymentCalendar(paymentCalendar)
            .withOvernightLegSpread(overnightSpread)
            .withAveragingMethod(averagingMethod);
        if (endOfMonth) {
            tmp.withEndOfMonth(*endOfMonth);
        }
        if (fixedPaymentFrequency) {
            tmp.withFixedLegPaymentFrequency(*fixedPaymentFrequency);
        }
        if (!fixedCalendar.empty()) {
            tmp.withFixedLegCalendar(fixedCalendar);
        }
        setInstrument<OvernightIndexedSwap>(tmp);
    }

    void DatedOISRateHelper::accept(AcyclicVisitor& v) {
        auto* v1 = dynamic_cast<Visitor<DatedOISRateHelper>*>(&v);
        if (v1 != nullptr)
            v1->visit(*this);
        else
            base_type::accept(v);
    }

}
