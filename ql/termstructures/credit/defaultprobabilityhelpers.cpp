/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008, 2009 Jose Aparicio
 Copyright (C) 2008 Chris Kenyon
 Copyright (C) 2008 Roland Lichters
 Copyright (C) 2008 StatPro Italia srl

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

#include <ql/termstructures/credit/defaultprobabilityhelpers.hpp>
#include <ql/pricingengines/credit/midpointcdsengine.hpp>
#include <ql/pricingengines/credit/isdacdsengine.hpp>
#include <ql/utilities/null_deleter.hpp>

namespace QuantLib {

    CdsHelper::CdsHelper(const Handle<Quote> &quote, const Period &tenor,
                         Integer settlementDays, const Calendar &calendar,
                         Frequency frequency,
                         BusinessDayConvention paymentConvention,
                         DateGeneration::Rule rule,
                         const DayCounter &dayCounter, Real recoveryRate,
                         const Handle<YieldTermStructure> &discountCurve,
                         bool settlesAccrual, bool paysAtDefaultTime,
                         const Date& startDate,
                         const DayCounter &lastPeriodDayCounter,
                         const bool rebatesAccrual,
                         const CreditDefaultSwap::PricingModel model)
        : RelativeDateDefaultProbabilityHelper(quote), tenor_(tenor),
          settlementDays_(settlementDays), calendar_(calendar),
          frequency_(frequency), paymentConvention_(paymentConvention),
          rule_(rule), dayCounter_(dayCounter), recoveryRate_(recoveryRate),
          discountCurve_(discountCurve), settlesAccrual_(settlesAccrual),
          paysAtDefaultTime_(paysAtDefaultTime), lastPeriodDC_(lastPeriodDayCounter),
          rebatesAccrual_(rebatesAccrual),
          model_(model), startDate_(startDate) {

        CdsHelper::initializeDates();

        registerWith(discountCurve);
    }

    CdsHelper::CdsHelper(Rate quote, const Period &tenor,
                         Integer settlementDays, const Calendar &calendar,
                         Frequency frequency,
                         BusinessDayConvention paymentConvention,
                         DateGeneration::Rule rule,
                         const DayCounter &dayCounter, Real recoveryRate,
                         const Handle<YieldTermStructure> &discountCurve,
                         bool settlesAccrual, bool paysAtDefaultTime,
                         const Date& startDate,
                         const DayCounter &lastPeriodDayCounter,
                         const bool rebatesAccrual,
                         const CreditDefaultSwap::PricingModel model)
        : RelativeDateDefaultProbabilityHelper(quote), tenor_(tenor),
          settlementDays_(settlementDays), calendar_(calendar),
          frequency_(frequency), paymentConvention_(paymentConvention),
          rule_(rule), dayCounter_(dayCounter), recoveryRate_(recoveryRate),
          discountCurve_(discountCurve), settlesAccrual_(settlesAccrual),
          paysAtDefaultTime_(paysAtDefaultTime),
          lastPeriodDC_(lastPeriodDayCounter), rebatesAccrual_(rebatesAccrual),
          model_(model), startDate_(startDate){

        CdsHelper::initializeDates();

        registerWith(discountCurve);
    }

    void CdsHelper::setTermStructure(DefaultProbabilityTermStructure* ts) {
        RelativeDateDefaultProbabilityHelper::setTermStructure(ts);

        probability_.linkTo(
            ext::shared_ptr<DefaultProbabilityTermStructure>(ts, null_deleter()),
            false);

        resetEngine();
    }

    void CdsHelper::update() {
        RelativeDateDefaultProbabilityHelper::update();
        resetEngine();
    }

    void CdsHelper::initializeDates() {
        protectionStart_ = evaluationDate_ + settlementDays_;
        Date startDate, endDate;
        if(startDate_ == Date()) {
            startDate = calendar_.adjust(protectionStart_,
                                         paymentConvention_);
            if (rule_ == DateGeneration::CDS || rule_ == DateGeneration::CDS2015) { // for standard CDS ..
                // .. the start date is not adjusted
                startDate = protectionStart_;
            }
            // .. and (in any case) the end date rolls by 3 month as
            //  soon as the trade date falls on an IMM date,
            // or the March or September IMM date in case of the CDS2015 rule.
            endDate = protectionStart_ + tenor_;

        } else {
            if(!schedule_.empty()) return; //no need to update schedule
            startDate = calendar_.adjust(startDate_, paymentConvention_);
            endDate = startDate_ + settlementDays_ + tenor_;
        }
        schedule_ =
            MakeSchedule().from(startDate)
                          .to(endDate)
                          .withFrequency(frequency_)
                          .withCalendar(calendar_)
                          .withConvention(paymentConvention_)
                          .withTerminationDateConvention(Unadjusted)
                          .withRule(rule_);
        earliestDate_ = schedule_.dates().front();
        latestDate_   = calendar_.adjust(schedule_.dates().back(),
                                         paymentConvention_);
        if (model_ == CreditDefaultSwap::ISDA)
            ++latestDate_;
    }

    SpreadCdsHelper::SpreadCdsHelper(
                              const Handle<Quote>& runningSpread,
                              const Period& tenor,
                              Integer settlementDays,
                              const Calendar& calendar,
                              Frequency frequency,
                              BusinessDayConvention paymentConvention,
                              DateGeneration::Rule rule,
                              const DayCounter& dayCounter,
                              Real recoveryRate,
                              const Handle<YieldTermStructure>& discountCurve,
                              bool settlesAccrual,
                              bool paysAtDefaultTime,
                              const Date& startDate,
                              const DayCounter& lastPeriodDayCounter,
                              const bool rebatesAccrual,
                              const CreditDefaultSwap::PricingModel model)
    : CdsHelper(runningSpread, tenor, settlementDays, calendar,
                frequency, paymentConvention, rule, dayCounter,
                recoveryRate, discountCurve, settlesAccrual, paysAtDefaultTime,
                startDate, lastPeriodDayCounter, rebatesAccrual, model) {}

    SpreadCdsHelper::SpreadCdsHelper(
                              Rate runningSpread,
                              const Period& tenor,
                              Integer settlementDays,
                              const Calendar& calendar,
                              Frequency frequency,
                              BusinessDayConvention paymentConvention,
                              DateGeneration::Rule rule,
                              const DayCounter& dayCounter,
                              Real recoveryRate,
                              const Handle<YieldTermStructure>& discountCurve,
                              bool settlesAccrual,
                              bool paysAtDefaultTime,
                              const Date& startDate,
                              const DayCounter& lastPeriodDayCounter,
                              const bool rebatesAccrual,
                              const CreditDefaultSwap::PricingModel model)
    : CdsHelper(runningSpread, tenor, settlementDays, calendar,
                frequency, paymentConvention, rule, dayCounter,
                recoveryRate, discountCurve, settlesAccrual, paysAtDefaultTime,
                startDate, lastPeriodDayCounter,rebatesAccrual, model) {}

    Real SpreadCdsHelper::impliedQuote() const {
        swap_->recalculate();
        return swap_->fairSpread();
    }

    void SpreadCdsHelper::resetEngine() {
        swap_ = ext::shared_ptr<CreditDefaultSwap>(new CreditDefaultSwap(
            Protection::Buyer, 100.0, 0.01, schedule_, paymentConvention_,
            dayCounter_, settlesAccrual_, paysAtDefaultTime_, protectionStart_,
            ext::shared_ptr<Claim>(), lastPeriodDC_, rebatesAccrual_));

        switch (model_) {
          case CreditDefaultSwap::ISDA:
            swap_->setPricingEngine(ext::make_shared<IsdaCdsEngine>(
                probability_, recoveryRate_, discountCurve_, false,
                IsdaCdsEngine::Taylor, IsdaCdsEngine::HalfDayBias,
                IsdaCdsEngine::Piecewise));
            break;
          case CreditDefaultSwap::Midpoint:
            swap_->setPricingEngine(ext::make_shared<MidPointCdsEngine>(
                probability_, recoveryRate_, discountCurve_));
            break;
          default:
            QL_FAIL("unknown CDS pricing model: " << model_);
        }
    }

    UpfrontCdsHelper::UpfrontCdsHelper(
                              const Handle<Quote>& upfront,
                              Rate runningSpread,
                              const Period& tenor,
                              Integer settlementDays,
                              const Calendar& calendar,
                              Frequency frequency,
                              BusinessDayConvention paymentConvention,
                              DateGeneration::Rule rule,
                              const DayCounter& dayCounter,
                              Real recoveryRate,
                              const Handle<YieldTermStructure>& discountCurve,
                              Natural upfrontSettlementDays,
                              bool settlesAccrual,
                              bool paysAtDefaultTime,
                              const Date& startDate,
                              const DayCounter& lastPeriodDayCounter,
                              const bool rebatesAccrual,
                              const CreditDefaultSwap::PricingModel model)
    : CdsHelper(upfront, tenor, settlementDays, calendar,
                frequency, paymentConvention, rule, dayCounter,
                recoveryRate, discountCurve, settlesAccrual, paysAtDefaultTime,
                startDate, lastPeriodDayCounter, rebatesAccrual, model),
      upfrontSettlementDays_(upfrontSettlementDays),
      runningSpread_(runningSpread) {
        UpfrontCdsHelper::initializeDates();
    }

    UpfrontCdsHelper::UpfrontCdsHelper(
                              Rate upfrontSpread,
                              Rate runningSpread,
                              const Period& tenor,
                              Integer settlementDays,
                              const Calendar& calendar,
                              Frequency frequency,
                              BusinessDayConvention paymentConvention,
                              DateGeneration::Rule rule,
                              const DayCounter& dayCounter,
                              Real recoveryRate,
                              const Handle<YieldTermStructure>& discountCurve,
                              Natural upfrontSettlementDays,
                              bool settlesAccrual,
                              bool paysAtDefaultTime,
                              const Date& startDate,
                              const DayCounter& lastPeriodDayCounter,
                              const bool rebatesAccrual,
                              const CreditDefaultSwap::PricingModel model)
    : CdsHelper(upfrontSpread, tenor, settlementDays, calendar,
                frequency, paymentConvention, rule, dayCounter,
                recoveryRate, discountCurve, settlesAccrual, paysAtDefaultTime,
                startDate, lastPeriodDayCounter, rebatesAccrual, model),
      upfrontSettlementDays_(upfrontSettlementDays),
      runningSpread_(runningSpread) {
        UpfrontCdsHelper::initializeDates();
    }

    void UpfrontCdsHelper::initializeDates() {
        CdsHelper::initializeDates();
        upfrontDate_ = calendar_.advance(evaluationDate_,
                                         upfrontSettlementDays_, Days,
                                         paymentConvention_);
    }

    void UpfrontCdsHelper::resetEngine() {
        swap_ = ext::shared_ptr<CreditDefaultSwap>(new CreditDefaultSwap(
            Protection::Buyer, 100.0, 0.01, runningSpread_, schedule_,
            paymentConvention_, dayCounter_, settlesAccrual_,
            paysAtDefaultTime_, protectionStart_, upfrontDate_,
            ext::shared_ptr<Claim>(), lastPeriodDC_, rebatesAccrual_));
        switch (model_) {
          case CreditDefaultSwap::ISDA:
            swap_->setPricingEngine(ext::make_shared<IsdaCdsEngine>(
                probability_, recoveryRate_, discountCurve_, false,
                IsdaCdsEngine::Taylor, IsdaCdsEngine::HalfDayBias,
                IsdaCdsEngine::Piecewise));
            break;
          case CreditDefaultSwap::Midpoint:
            swap_->setPricingEngine(ext::make_shared<MidPointCdsEngine>(
                probability_, recoveryRate_, discountCurve_));
            break;
          default:
            QL_FAIL("unknown CDS pricing model: " << model_);
        }
    }

    Real UpfrontCdsHelper::impliedQuote() const {
        SavedSettings backup;
        Settings::instance().includeTodaysCashFlows() = true;
        swap_->recalculate();
        return swap_->fairUpfront();
    }

}
