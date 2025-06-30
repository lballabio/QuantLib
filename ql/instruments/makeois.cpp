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
#include <ql/time/schedule.hpp>
#include <ql/indexes/ibor/sonia.hpp>
#include <ql/indexes/ibor/corra.hpp>

namespace QuantLib {

    MakeOIS::MakeOIS(const Period& swapTenor,
                     const ext::shared_ptr<OvernightIndex>& overnightIndex,
                     Rate fixedRate,
                     const Period& forwardStart)
    : swapTenor_(swapTenor), overnightIndex_(overnightIndex), fixedRate_(fixedRate),
      forwardStart_(forwardStart),
      settlementDays_(Null<Natural>()),
      fixedCalendar_(overnightIndex->fixingCalendar()),
      overnightCalendar_(overnightIndex->fixingCalendar()),
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
            // settlement days: override if set, else fallback to default by index name
            Natural settlementDays = settlementDays_;
            if (settlementDays == Null<Natural>()) {
                if (ext::dynamic_pointer_cast<Sonia>(overnightIndex_)) {
                    settlementDays = 0; 
                }
                else if (ext::dynamic_pointer_cast<Corra>(overnightIndex_)) {
                    settlementDays = 1;
                }
                else {
                    settlementDays = 2;
                }
            }            

            Date refDate = Settings::instance().evaluationDate();
            // if the evaluation date is not a business day
            // then move to the next business day
            refDate = overnightCalendar_.adjust(refDate);
            Date spotDate = overnightCalendar_.advance(refDate,
                                                       settlementDays*Days);
            startDate = spotDate+forwardStart_;
            if (forwardStart_.length()<0)
                startDate = overnightCalendar_.adjust(startDate, Preceding);
            else
                startDate = overnightCalendar_.adjust(startDate, Following);
        }

        // OIS end of month default
        bool fixedEndOfMonth, overnightEndOfMonth;
        if (isDefaultEOM_)
            fixedEndOfMonth = overnightEndOfMonth = overnightCalendar_.isEndOfMonth(startDate);
        else {
            fixedEndOfMonth = fixedEndOfMonth_;
            overnightEndOfMonth = overnightEndOfMonth_;
        }

        Date endDate = terminationDate_;
        if (endDate == Date()) {
            if (overnightEndOfMonth)
                endDate = overnightCalendar_.advance(startDate,
                                                     swapTenor_,
                                                     ModifiedFollowing,
                                                     overnightEndOfMonth);
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
                               fixedCalendar_,
                               fixedConvention_,
                               fixedTerminationDateConvention_,
                               fixedRule,
                               fixedEndOfMonth);

        Schedule overnightSchedule(startDate, endDate,
                                   Period(overnightPaymentFrequency),
                                   overnightCalendar_,
                                   overnightConvention_,
                                   overnightTerminationDateConvention_,
                                   overnightRule,
                                   overnightEndOfMonth);

        Rate usedFixedRate = fixedRate_;
        if (fixedRate_ == Null<Rate>()) {
            OvernightIndexedSwap temp(type_, nominal_,
                                      fixedSchedule,
                                      0.0, // fixed rate
                                      fixedDayCount_,
                                      overnightSchedule,
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
                                 overnightSchedule,
                                 overnightIndex_, overnightSpread_,
                                 paymentLag_, paymentAdjustment_,
                                 paymentCalendar_, telescopicValueDates_, 
                                 averagingMethod_, lookbackDays_,
                                 lockoutDays_, applyObservationShift_));

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
        if (terminationDate != Date())
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

    MakeOIS& MakeOIS::withCalendar(const Calendar& cal) {
        return withFixedLegCalendar(cal).withOvernightLegCalendar(cal);
    }

    MakeOIS& MakeOIS::withFixedLegCalendar(const Calendar& cal) {
        fixedCalendar_ = cal;
        return *this;
    }

    MakeOIS& MakeOIS::withOvernightLegCalendar(const Calendar& cal) {
        overnightCalendar_ = cal;
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

    MakeOIS& MakeOIS::withConvention(BusinessDayConvention bdc) {
        return withFixedLegConvention(bdc).withOvernightLegConvention(bdc);
    }

    MakeOIS& MakeOIS::withFixedLegConvention(BusinessDayConvention bdc) {
        fixedConvention_ = bdc;
        return *this;
    }

    MakeOIS& MakeOIS::withOvernightLegConvention(BusinessDayConvention bdc) {
        overnightConvention_ = bdc;
        return *this;
    }

    MakeOIS& MakeOIS::withTerminationDateConvention(BusinessDayConvention bdc) {
        withFixedLegTerminationDateConvention(bdc);
        return withOvernightLegTerminationDateConvention(bdc);
    }

    MakeOIS& MakeOIS::withFixedLegTerminationDateConvention(BusinessDayConvention bdc) {
        fixedTerminationDateConvention_ = bdc;
        return *this;
    }

    MakeOIS& MakeOIS::withOvernightLegTerminationDateConvention(BusinessDayConvention bdc) {
        overnightTerminationDateConvention_ = bdc;
        return *this;
    }

    MakeOIS& MakeOIS::withEndOfMonth(bool flag) {
        return withFixedLegEndOfMonth(flag).withOvernightLegEndOfMonth(flag);
    }

    MakeOIS& MakeOIS::withFixedLegEndOfMonth(bool flag) {
        fixedEndOfMonth_ = flag;
        isDefaultEOM_ = false;
        return *this;
    }

    MakeOIS& MakeOIS::withOvernightLegEndOfMonth(bool flag) {
        overnightEndOfMonth_ = flag;
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

    MakeOIS& MakeOIS::withLookbackDays(Natural lookbackDays) {
        lookbackDays_ = lookbackDays;
        return *this;
    }

    MakeOIS& MakeOIS::withLockoutDays(Natural lockoutDays) {
        lockoutDays_ = lockoutDays;
        return *this;
    }

    MakeOIS& MakeOIS::withObservationShift(bool applyObservationShift) {
        applyObservationShift_ = applyObservationShift;
        return *this;
    }

}
