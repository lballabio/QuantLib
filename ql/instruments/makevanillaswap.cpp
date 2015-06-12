/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006, 2007, 2010, 2014l, 2015 Ferdinando Ametrano
 Copyright (C) 2006 Katiuscia Manzoni
 Copyright (C) 2006 StatPro Italia srl
 Copyright (C) 2015 Paolo Mazzocchi

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

#include <ql/instruments/makevanillaswap.hpp>
#include <ql/pricingengines/swap/discountingswapengine.hpp>
#include <ql/time/daycounters/thirty360.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>
#include <ql/indexes/iborindex.hpp>
#include <ql/time/schedule.hpp>
#include <ql/currencies/america.hpp>
#include <ql/currencies/asia.hpp>
#include <ql/currencies/europe.hpp>
#include <ql/currencies/oceania.hpp>

using boost::shared_ptr;

namespace QuantLib {

    MakeVanillaSwap::MakeVanillaSwap(const Period& swapTenor,
                                     const shared_ptr<IborIndex>& index,
                                     Rate fixedRate,
                                     const Period& forwardStart)
    : swapTenor_(swapTenor), iborIndex_(index),
      fixedRate_(fixedRate), forwardStart_(forwardStart),
      settlementDays_(iborIndex_->fixingDays()),
      fixedCalendar_(index->fixingCalendar()),
      floatCalendar_(index->fixingCalendar()),
      type_(VanillaSwap::Payer), nominal_(1.0),
      floatTenor_(index->tenor()),
      fixedConvention_(ModifiedFollowing),
      fixedTerminationDateConvention_(ModifiedFollowing),
      floatConvention_(index->businessDayConvention()),
      floatTerminationDateConvention_(index->businessDayConvention()),
      fixedRule_(DateGeneration::Backward), floatRule_(DateGeneration::Backward),
      fixedEndOfMonth_(false), floatEndOfMonth_(false),
      fixedFirstDate_(Date()), fixedNextToLastDate_(Date()),
      floatFirstDate_(Date()), floatNextToLastDate_(Date()),
      floatSpread_(0.0),
      floatDayCount_(index->dayCounter()) {}

    MakeVanillaSwap::operator VanillaSwap() const {
        shared_ptr<VanillaSwap> swap = *this;
        return *swap;
    }

    MakeVanillaSwap::operator shared_ptr<VanillaSwap>() const {

        Date startDate;
        if (effectiveDate_ != Date())
            startDate = effectiveDate_;
        else {
            Date refDate = Settings::instance().evaluationDate();
            // if the evaluation date is not a business day
            // then move to the next business day
            refDate = floatCalendar_.adjust(refDate);
            Date spotDate = floatCalendar_.advance(refDate,
                                                   settlementDays_*Days);
            startDate = spotDate+forwardStart_;
            if (forwardStart_.length()<0)
                startDate = floatCalendar_.adjust(startDate,
                                                  Preceding);
            else
                startDate = floatCalendar_.adjust(startDate,
                                                  Following);
        }

        Date endDate = terminationDate_;
        if (endDate == Date()) {
            if (floatEndOfMonth_)
                endDate = floatCalendar_.advance(startDate,
                                                 swapTenor_,
                                                 ModifiedFollowing,
                                                 floatEndOfMonth_);
            else
                endDate = startDate + swapTenor_;
        }

        const Currency& curr = iborIndex_->currency();
        Period fixedTenor;
        if (fixedTenor_ != Period())
            fixedTenor = fixedTenor_;
        else {
            if ((curr == EURCurrency()) ||
                (curr == USDCurrency()) ||
                (curr == CHFCurrency()) ||
                (curr == SEKCurrency()) ||
                (curr == GBPCurrency() && swapTenor_ <= 1 * Years))
                fixedTenor = Period(1, Years);
            else if ((curr == GBPCurrency() && swapTenor_ > 1 * Years) ||
                (curr == JPYCurrency()) ||
                (curr == AUDCurrency() && swapTenor_ >= 4 * Years))
                fixedTenor = Period(6, Months);
            else if ((curr == HKDCurrency() ||
                     (curr == AUDCurrency() && swapTenor_ < 4 * Years)))
                fixedTenor = Period(3, Months);
            else
                QL_FAIL("unknown fixed leg default tenor for " << curr);
        }

        Schedule fixedSchedule(startDate, endDate,
                               fixedTenor, fixedCalendar_,
                               fixedConvention_,
                               fixedTerminationDateConvention_,
                               fixedRule_, fixedEndOfMonth_,
                               fixedFirstDate_, fixedNextToLastDate_);

        Schedule floatSchedule(startDate, endDate,
                               floatTenor_, floatCalendar_,
                               floatConvention_,
                               floatTerminationDateConvention_,
                               floatRule_, floatEndOfMonth_,
                               floatFirstDate_, floatNextToLastDate_);

        DayCounter fixedDayCount;
        if (fixedDayCount_ != DayCounter())
            fixedDayCount = fixedDayCount_;
        else {
            if (curr == USDCurrency())
                fixedDayCount = Actual360();
            else if (curr == EURCurrency() || curr == CHFCurrency() ||
                     curr == SEKCurrency())
                fixedDayCount = Thirty360(Thirty360::BondBasis);
            else if (curr == GBPCurrency() || curr == JPYCurrency() ||
                     curr == AUDCurrency() || curr == HKDCurrency())
                fixedDayCount = Actual365Fixed();
            else
                QL_FAIL("unknown fixed leg day counter for " << curr);
        }

        Rate usedFixedRate = fixedRate_;
        if (fixedRate_ == Null<Rate>()) {
            VanillaSwap temp(type_, nominal_,
                             fixedSchedule,
                             0.0, // fixed rate
                             fixedDayCount,
                             floatSchedule, iborIndex_,
                             floatSpread_, floatDayCount_);
            if (engine_ == 0) {
                Handle<YieldTermStructure> disc =
                                        iborIndex_->forwardingTermStructure();
                QL_REQUIRE(!disc.empty(),
                           "null term structure set to this instance of " <<
                           iborIndex_->name());
                bool includeSettlementDateFlows = false;
                shared_ptr<PricingEngine> engine(new
                    DiscountingSwapEngine(disc, includeSettlementDateFlows));
                temp.setPricingEngine(engine);
            } else
                temp.setPricingEngine(engine_);

            usedFixedRate = temp.fairRate();
        }

        shared_ptr<VanillaSwap> swap(new
            VanillaSwap(type_, nominal_,
                        fixedSchedule,
                        usedFixedRate, fixedDayCount,
                        floatSchedule,
                        iborIndex_, floatSpread_, floatDayCount_));

        if (engine_ == 0) {
            Handle<YieldTermStructure> disc =
                                    iborIndex_->forwardingTermStructure();
            bool includeSettlementDateFlows = false;
            shared_ptr<PricingEngine> engine(new
                DiscountingSwapEngine(disc, includeSettlementDateFlows));
            swap->setPricingEngine(engine);
        } else
            swap->setPricingEngine(engine_);

        return swap;
    }

    MakeVanillaSwap& MakeVanillaSwap::receiveFixed(bool flag) {
        type_ = flag ? VanillaSwap::Receiver : VanillaSwap::Payer ;
        return *this;
    }

    MakeVanillaSwap& MakeVanillaSwap::withType(VanillaSwap::Type type) {
        type_ = type;
        return *this;
    }

    MakeVanillaSwap& MakeVanillaSwap::withNominal(Real n) {
        nominal_ = n;
        return *this;
    }

    MakeVanillaSwap& MakeVanillaSwap::withSettlementDays(Natural settlementDays) {
        settlementDays_ = settlementDays;
        effectiveDate_ = Date();
        return *this;
    }

    MakeVanillaSwap&
    MakeVanillaSwap::withEffectiveDate(const Date& effectiveDate) {
        effectiveDate_ = effectiveDate;
        return *this;
    }

    MakeVanillaSwap&
    MakeVanillaSwap::withTerminationDate(const Date& terminationDate) {
        terminationDate_ = terminationDate;
        swapTenor_ = Period();
        return *this;
    }

    MakeVanillaSwap& MakeVanillaSwap::withRule(DateGeneration::Rule r) {
        fixedRule_ = r;
        floatRule_ = r;
        return *this;
    }

    MakeVanillaSwap& MakeVanillaSwap::withDiscountingTermStructure(
                                        const Handle<YieldTermStructure>& d) {
        bool includeSettlementDateFlows = false;
        engine_ = shared_ptr<PricingEngine>(new
            DiscountingSwapEngine(d, includeSettlementDateFlows));
        return *this;
    }

    MakeVanillaSwap& MakeVanillaSwap::withPricingEngine(
                             const shared_ptr<PricingEngine>& engine) {
        engine_ = engine;
        return *this;
    }

    MakeVanillaSwap& MakeVanillaSwap::withFixedLegTenor(const Period& t) {
        fixedTenor_ = t;
        return *this;
    }

    MakeVanillaSwap&
    MakeVanillaSwap::withFixedLegCalendar(const Calendar& cal) {
        fixedCalendar_ = cal;
        return *this;
    }

    MakeVanillaSwap&
    MakeVanillaSwap::withFixedLegConvention(BusinessDayConvention bdc) {
        fixedConvention_ = bdc;
        return *this;
    }

    MakeVanillaSwap&
    MakeVanillaSwap::withFixedLegTerminationDateConvention(BusinessDayConvention bdc) {
        fixedTerminationDateConvention_ = bdc;
        return *this;
    }

    MakeVanillaSwap& MakeVanillaSwap::withFixedLegRule(DateGeneration::Rule r) {
        fixedRule_ = r;
        return *this;
    }

    MakeVanillaSwap& MakeVanillaSwap::withFixedLegEndOfMonth(bool flag) {
        fixedEndOfMonth_ = flag;
        return *this;
    }

    MakeVanillaSwap& MakeVanillaSwap::withFixedLegFirstDate(const Date& d) {
        fixedFirstDate_ = d;
        return *this;
    }

    MakeVanillaSwap&
    MakeVanillaSwap::withFixedLegNextToLastDate(const Date& d) {
        fixedNextToLastDate_ = d;
        return *this;
    }

    MakeVanillaSwap&
    MakeVanillaSwap::withFixedLegDayCount(const DayCounter& dc) {
        fixedDayCount_ = dc;
        return *this;
    }

    MakeVanillaSwap& MakeVanillaSwap::withFloatingLegTenor(const Period& t) {
        floatTenor_ = t;
        return *this;
    }

    MakeVanillaSwap&
    MakeVanillaSwap::withFloatingLegCalendar(const Calendar& cal) {
        floatCalendar_ = cal;
        return *this;
    }

    MakeVanillaSwap&
    MakeVanillaSwap::withFloatingLegConvention(BusinessDayConvention bdc) {
        floatConvention_ = bdc;
        return *this;
    }

    MakeVanillaSwap&
    MakeVanillaSwap::withFloatingLegTerminationDateConvention(BusinessDayConvention bdc) {
        floatTerminationDateConvention_ = bdc;
        return *this;
    }

    MakeVanillaSwap& MakeVanillaSwap::withFloatingLegRule(DateGeneration::Rule r) {
        floatRule_ = r;
        return *this;
    }

    MakeVanillaSwap& MakeVanillaSwap::withFloatingLegEndOfMonth(bool flag) {
        floatEndOfMonth_ = flag;
        return *this;
    }

    MakeVanillaSwap&
    MakeVanillaSwap::withFloatingLegFirstDate(const Date& d) {
        floatFirstDate_ = d;
        return *this;
    }

    MakeVanillaSwap&
    MakeVanillaSwap::withFloatingLegNextToLastDate(const Date& d) {
        floatNextToLastDate_ = d;
        return *this;
    }

    MakeVanillaSwap&
    MakeVanillaSwap::withFloatingLegDayCount(const DayCounter& dc) {
        floatDayCount_ = dc;
        return *this;
    }

    MakeVanillaSwap& MakeVanillaSwap::withFloatingLegSpread(Spread sp) {
        floatSpread_ = sp;
        return *this;
    }

}
