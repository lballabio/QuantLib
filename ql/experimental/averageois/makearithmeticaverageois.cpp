/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2016 Stefano Fondi

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

#include <ql/experimental/averageois/makearithmeticaverageois.hpp>
#include <ql/pricingengines/swap/discountingswapengine.hpp>
#include <ql/indexes/iborindex.hpp>
#include <ql/time/schedule.hpp>

namespace QuantLib {

    QL_DEPRECATED_DISABLE_WARNING

    MakeArithmeticAverageOIS::MakeArithmeticAverageOIS(
        const Period& swapTenor,
        const ext::shared_ptr<OvernightIndex>& overnightIndex,
        Rate fixedRate,
        const Period& forwardStart)
    : swapTenor_(swapTenor), overnightIndex_(overnightIndex), fixedRate_(fixedRate),
      forwardStart_(forwardStart),

      calendar_(overnightIndex->fixingCalendar()),

      fixedDayCount_(overnightIndex->dayCounter()) {}

    MakeArithmeticAverageOIS::operator ArithmeticAverageOIS() const {
        ext::shared_ptr<ArithmeticAverageOIS> ois = *this;
        return *ois;
    }

    MakeArithmeticAverageOIS::operator ext::shared_ptr<ArithmeticAverageOIS>() const {

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

        Schedule fixedLegSchedule(startDate, endDate,
                          Period(fixedLegPaymentFrequency_),
                          calendar_,
                          ModifiedFollowing,
                          ModifiedFollowing,
                          rule_,
                          usedEndOfMonth);

        Schedule overnightLegSchedule(startDate, endDate,
                          Period(overnightLegPaymentFrequency_),
                          calendar_,
                          ModifiedFollowing,
                          ModifiedFollowing,
                          rule_,
                          usedEndOfMonth);

        Rate usedFixedRate = fixedRate_;
        if (fixedRate_ == Null<Rate>()) {
            ArithmeticAverageOIS temp(type_, nominal_,
                                      fixedLegSchedule,
                                      0.0, // fixed rate
                                      fixedDayCount_,
                                      overnightIndex_,
                                      overnightLegSchedule,
                                      overnightSpread_,
                                      mrs_, vol_, byApprox_);
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

        ext::shared_ptr<ArithmeticAverageOIS> ois(new
            ArithmeticAverageOIS(type_, nominal_,
                                 fixedLegSchedule,
                                 usedFixedRate, fixedDayCount_,
                                 overnightIndex_,
                                 overnightLegSchedule,
                                 overnightSpread_,
                                 mrs_, vol_, byApprox_));

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

    MakeArithmeticAverageOIS& MakeArithmeticAverageOIS::receiveFixed(bool flag) {
        type_ = flag ? Swap::Receiver : Swap::Payer;
        return *this;
    }

    MakeArithmeticAverageOIS& MakeArithmeticAverageOIS::withType(Swap::Type type) {
        type_ = type;
        return *this;
    }

    MakeArithmeticAverageOIS& MakeArithmeticAverageOIS::withNominal(Real n) {
        nominal_ = n;
        return *this;
    }

    MakeArithmeticAverageOIS& MakeArithmeticAverageOIS::withSettlementDays(Natural settlementDays) {
        settlementDays_ = settlementDays;
        effectiveDate_ = Date();
        return *this;
    }

    MakeArithmeticAverageOIS& MakeArithmeticAverageOIS::withEffectiveDate(const Date& effectiveDate) {
        effectiveDate_ = effectiveDate;
        return *this;
    }

    MakeArithmeticAverageOIS& MakeArithmeticAverageOIS::withTerminationDate(const Date& terminationDate) {
        terminationDate_ = terminationDate;
        swapTenor_ = Period();
        return *this;
    }

    MakeArithmeticAverageOIS& MakeArithmeticAverageOIS::withFixedLegPaymentFrequency(Frequency f) {
        fixedLegPaymentFrequency_ = f;
        if (fixedLegPaymentFrequency_ == Once)
            rule_ = DateGeneration::Zero;
        return *this;
    }

    MakeArithmeticAverageOIS& MakeArithmeticAverageOIS::withOvernightLegPaymentFrequency(Frequency f) {
        overnightLegPaymentFrequency_ = f;
        if (overnightLegPaymentFrequency_ == Once)
            rule_ = DateGeneration::Zero;
        return *this;
    }

    MakeArithmeticAverageOIS& MakeArithmeticAverageOIS::withRule(DateGeneration::Rule r) {
        rule_ = r;
        if (r==DateGeneration::Zero) {
            fixedLegPaymentFrequency_ = Once;
            overnightLegPaymentFrequency_ = Once;
        }
        return *this;
    }

    MakeArithmeticAverageOIS& MakeArithmeticAverageOIS::withDiscountingTermStructure(
                                        const Handle<YieldTermStructure>& d) {
        bool includeSettlementDateFlows = false;
        engine_ = ext::shared_ptr<PricingEngine>(new
            DiscountingSwapEngine(d, includeSettlementDateFlows));
        return *this;
    }

    MakeArithmeticAverageOIS& MakeArithmeticAverageOIS::withPricingEngine(
                             const ext::shared_ptr<PricingEngine>& engine) {
        engine_ = engine;
        return *this;
    }

    MakeArithmeticAverageOIS& MakeArithmeticAverageOIS::withFixedLegDayCount(const DayCounter& dc) {
        fixedDayCount_ = dc;
        return *this;
    }

    MakeArithmeticAverageOIS& MakeArithmeticAverageOIS::withEndOfMonth(bool flag) {
        endOfMonth_ = flag;
        isDefaultEOM_ = false;
        return *this;
    }

    MakeArithmeticAverageOIS& MakeArithmeticAverageOIS::withOvernightLegSpread(Spread sp) {
        overnightSpread_ = sp;
        return *this;
    }

    MakeArithmeticAverageOIS& MakeArithmeticAverageOIS::withArithmeticAverage(
                                            Real meanReversionSpeed,
                                            Real volatility,
                                            bool byApprox) {
        mrs_ = meanReversionSpeed;
        vol_ = volatility;
        byApprox_ = byApprox;
        return *this;
    }

    QL_DEPRECATED_ENABLE_WARNING

}
