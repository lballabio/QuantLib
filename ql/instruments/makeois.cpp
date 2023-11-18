/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009, 2014, 2015 Ferdinando Ametrano
 Copyright (C) 2015 Paolo Mazzocchi
 Copyright (C) 2017 Joseph Jeisman
 Copyright (C) 2017 Fabrice Lecuyer

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
#include <ql/pricingengines/swap/discountingswapengine.hpp>
#include <ql/indexes/iborindex.hpp>
#include <ql/optional.hpp>
#include <ql/time/schedule.hpp>

namespace QuantLib {

    MakeOIS::MakeOIS(const Period& swapTenor,
                     const ext::shared_ptr<OvernightIndex>& overnightIndex,
                     Rate fixedRate,
                     const Period& forwardStart)
    : swapTenor_(swapTenor), overnightIndex_(overnightIndex), fixedRate_(fixedRate),
      forwardStart_(forwardStart),

      calendar_(overnightIndex->fixingCalendar()),

      fixedDayCount_(overnightIndex->dayCounter()) {}

    MakeOIS::operator OvernightIndexedSwap() const {
        ext::shared_ptr<OvernightIndexedSwap> ois = *this;
        return *ois;
    }

    MakeOIS::operator ext::shared_ptr<OvernightIndexedSwap>() const {

        Date startDate;
        if (effectiveDate_ != Date())
            startDate = effectiveDate_;
        else {
            Date refDate = Settings::instance().evaluationDate();
            // if the evaluation date is not a business day
            // then move to the next business day
            refDate = calendar_.adjust(refDate);
            Date spotDate = calendar_.advance(refDate,
                                              settlementDays_*Days);
            startDate = spotDate+forwardStart_;
            if (forwardStart_.length()<0)
                startDate = calendar_.adjust(startDate, Preceding);
            else
                startDate = calendar_.adjust(startDate, Following);
        }

        // OIS end of month default
        bool usedEndOfMonth =
            isDefaultEOM_ ? calendar_.isEndOfMonth(startDate) : endOfMonth_;

        Date endDate = terminationDate_;
        if (endDate == Date()) {
            if (usedEndOfMonth)
                endDate = calendar_.advance(startDate,
                                            swapTenor_,
                                            ModifiedFollowing,
                                            usedEndOfMonth);
            else
                endDate = startDate + swapTenor_;
        }

        Frequency fixedPaymentFrequency, overnightPaymentFrequency;
        DateGeneration::Rule fixedRule, overnightRule;
        if (fixedPaymentFrequency_ == Once || fixedRule_ == DateGeneration::Zero) {
            fixedPaymentFrequency = Once;
            fixedRule = DateGeneration::Zero;
        } else {
            fixedPaymentFrequency = fixedPaymentFrequency_;
            fixedRule = fixedRule_;
        }
        if (overnightPaymentFrequency_ == Once || overnightRule_ == DateGeneration::Zero) {
            overnightPaymentFrequency = Once;
            overnightRule = DateGeneration::Zero;
        } else {
            overnightPaymentFrequency = overnightPaymentFrequency_;
            overnightRule = overnightRule_;
        }

        Schedule fixedSchedule(startDate, endDate,
                               Period(fixedPaymentFrequency),
                               calendar_,
                               ModifiedFollowing,
                               ModifiedFollowing,
                               fixedRule,
                               usedEndOfMonth);
        ext::optional<Schedule> overnightSchedule;
        if (fixedPaymentFrequency != overnightPaymentFrequency || fixedRule != overnightRule) {
            overnightSchedule.emplace(startDate, endDate,
                                      Period(overnightPaymentFrequency),
                                      calendar_,
                                      ModifiedFollowing,
                                      ModifiedFollowing,
                                      overnightRule,
                                      usedEndOfMonth);
        }

        Rate usedFixedRate = fixedRate_;
        if (fixedRate_ == Null<Rate>()) {
            OvernightIndexedSwap temp(type_, nominal_,
                                      fixedSchedule,
                                      0.0, // fixed rate
                                      fixedDayCount_,
                                      overnightSchedule ? *overnightSchedule : fixedSchedule,
                                      overnightIndex_, overnightSpread_,
                                      paymentLag_, paymentAdjustment_,
                                      paymentCalendar_, telescopicValueDates_);
            if (engine_ == nullptr) {
                Handle<YieldTermStructure> disc =
                                    overnightIndex_->forwardingTermStructure();
                QL_REQUIRE(!disc.empty(),
                           "null term structure set to this instance of " <<
                           overnightIndex_->name());
                bool includeSettlementDateFlows = false;
                ext::shared_ptr<PricingEngine> engine(new
                    DiscountingSwapEngine(disc, includeSettlementDateFlows));
                temp.setPricingEngine(engine);
            } else
                temp.setPricingEngine(engine_);

            usedFixedRate = temp.fairRate();
        }

        ext::shared_ptr<OvernightIndexedSwap> ois(new
            OvernightIndexedSwap(type_, nominal_,
                                 fixedSchedule,
                                 usedFixedRate, fixedDayCount_,
                                 overnightSchedule ? *overnightSchedule : fixedSchedule,
                                 overnightIndex_, overnightSpread_,
                                 paymentLag_, paymentAdjustment_,
                                 paymentCalendar_, telescopicValueDates_, 
                                 averagingMethod_));

        if (engine_ == nullptr) {
            Handle<YieldTermStructure> disc =
                                overnightIndex_->forwardingTermStructure();
            bool includeSettlementDateFlows = false;
            ext::shared_ptr<PricingEngine> engine(new
                DiscountingSwapEngine(disc, includeSettlementDateFlows));
            ois->setPricingEngine(engine);
        } else
            ois->setPricingEngine(engine_);

        return ois;
    }

    MakeOIS& MakeOIS::receiveFixed(bool flag) {
        type_ = flag ? Swap::Receiver : Swap::Payer ;
        return *this;
    }

    MakeOIS& MakeOIS::withType(Swap::Type type) {
        type_ = type;
        return *this;
    }

    MakeOIS& MakeOIS::withNominal(Real n) {
        nominal_ = n;
        return *this;
    }

    MakeOIS& MakeOIS::withSettlementDays(Natural settlementDays) {
        settlementDays_ = settlementDays;
        effectiveDate_ = Date();
        return *this;
    }

    MakeOIS& MakeOIS::withEffectiveDate(const Date& effectiveDate) {
        effectiveDate_ = effectiveDate;
        return *this;
    }

    MakeOIS& MakeOIS::withTerminationDate(const Date& terminationDate) {
        terminationDate_ = terminationDate;
        swapTenor_ = Period();
        return *this;
    }

    MakeOIS& MakeOIS::withPaymentFrequency(Frequency f) {
        return withFixedLegPaymentFrequency(f).withOvernightLegPaymentFrequency(f);
    }

    MakeOIS& MakeOIS::withFixedLegPaymentFrequency(Frequency f) {
        fixedPaymentFrequency_ = f;
        return *this;
    }

    MakeOIS& MakeOIS::withOvernightLegPaymentFrequency(Frequency f) {
        overnightPaymentFrequency_ = f;
        return *this;
    }

    MakeOIS& MakeOIS::withPaymentAdjustment(BusinessDayConvention convention) {
        paymentAdjustment_ = convention;
        return *this;
    }

    MakeOIS& MakeOIS::withPaymentLag(Integer lag) {
        paymentLag_ = lag;
        return *this;
    }

    MakeOIS& MakeOIS::withPaymentCalendar(const Calendar& cal) {
        paymentCalendar_ = cal;
        return *this;
    }

    MakeOIS& MakeOIS::withRule(DateGeneration::Rule r) {
        return withFixedLegRule(r).withOvernightLegRule(r);
    }

    MakeOIS& MakeOIS::withFixedLegRule(DateGeneration::Rule r) {
        fixedRule_ = r;
        return *this;
    }

    MakeOIS& MakeOIS::withOvernightLegRule(DateGeneration::Rule r) {
        overnightRule_ = r;
        return *this;
    }

    MakeOIS& MakeOIS::withDiscountingTermStructure(
                                        const Handle<YieldTermStructure>& d) {
        bool includeSettlementDateFlows = false;
        engine_ = ext::shared_ptr<PricingEngine>(new
            DiscountingSwapEngine(d, includeSettlementDateFlows));
        return *this;
    }

    MakeOIS& MakeOIS::withPricingEngine(
                             const ext::shared_ptr<PricingEngine>& engine) {
        engine_ = engine;
        return *this;
    }

    MakeOIS& MakeOIS::withFixedLegDayCount(const DayCounter& dc) {
        fixedDayCount_ = dc;
        return *this;
    }

    MakeOIS& MakeOIS::withEndOfMonth(bool flag) {
        endOfMonth_ = flag;
        isDefaultEOM_ = false;
        return *this;
    }

    MakeOIS& MakeOIS::withOvernightLegSpread(Spread sp) {
        overnightSpread_ = sp;
        return *this;
    }

    MakeOIS& MakeOIS::withTelescopicValueDates(bool telescopicValueDates) {
        telescopicValueDates_ = telescopicValueDates;
        return *this;
    }

    MakeOIS& MakeOIS::withAveragingMethod(RateAveraging::Type averagingMethod) {
        averagingMethod_ = averagingMethod;
        return *this;
    }

}
