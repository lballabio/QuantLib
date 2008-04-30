/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Jose Aparicio
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
#include <ql/instruments/creditdefaultswap.hpp>
#include <ql/pricingengines/credit/midpointcdsengine.hpp>

namespace QuantLib {

    namespace {
        void no_deletion(DefaultProbabilityTermStructure*) {}
    }

    CdsHelper::CdsHelper(const Handle<Quote>& spread,
                         const Period& tenor,
                         Integer settlementDays,
                         const Calendar& calendar,
                         Frequency frequency,
                         BusinessDayConvention paymentConvention,
                         const DayCounter& dayCounter,
                         Real recoveryRate,
                         const Handle<YieldTermStructure>& discountCurve,
                         bool settlesAccrual,
                         bool paysAtDefaultTime)
    : DefaultProbabilityHelper(spread),
      tenor_(tenor), settlementDays_(settlementDays), calendar_(calendar),
      frequency_(frequency), paymentConvention_(paymentConvention),
      dayCounter_(dayCounter), recoveryRate_(recoveryRate),
      discountCurve_(discountCurve),
      settlesAccrual_(settlesAccrual), paysAtDefaultTime_(paysAtDefaultTime) {

        initializeDates();

        registerWith(Settings::instance().evaluationDate());
        registerWith(discountCurve);
    }

    CdsHelper::CdsHelper(Rate spread,
                         const Period& tenor,
                         Integer settlementDays,
                         const Calendar& calendar,
                         Frequency frequency,
                         BusinessDayConvention paymentConvention,
                         const DayCounter& dayCounter,
                         Real recoveryRate,
                         const Handle<YieldTermStructure>& discountCurve,
                         bool settlesAccrual,
                         bool paysAtDefaultTime)
    : DefaultProbabilityHelper(spread),
      tenor_(tenor), settlementDays_(settlementDays), calendar_(calendar),
      frequency_(frequency), paymentConvention_(paymentConvention),
      dayCounter_(dayCounter), recoveryRate_(recoveryRate),
      discountCurve_(discountCurve),
      settlesAccrual_(settlesAccrual), paysAtDefaultTime_(paysAtDefaultTime) {

        initializeDates();

        registerWith(Settings::instance().evaluationDate());
        registerWith(discountCurve);
    }


    Real CdsHelper::impliedQuote() const {
        swap_->recalculate();
        return swap_->fairSpread();
    }

    void CdsHelper::setTermStructure(DefaultProbabilityTermStructure* ts) {
        DefaultProbabilityHelper::setTermStructure(ts);
        
        probability_.linkTo(
            boost::shared_ptr<DefaultProbabilityTermStructure>(ts, no_deletion),
            false);

        initializeDates();
    }

    void CdsHelper::update() {
        if (evaluationDate_ != Settings::instance().evaluationDate())
            initializeDates();

        DefaultProbabilityHelper::update();
    }

    void CdsHelper::initializeDates() {
        evaluationDate_ = Settings::instance().evaluationDate();

        Date startDate = calendar_.advance(evaluationDate_,
                                           settlementDays_, Days);
        Date endDate = calendar_.adjust(startDate + tenor_, paymentConvention_);

        Schedule schedule = MakeSchedule(startDate, endDate,
                                         Period(frequency_),
                                         calendar_,
                                         paymentConvention_);
        earliestDate_ = schedule.dates().front();
        latestDate_ = schedule.dates().back();

        swap_ = boost::shared_ptr<CreditDefaultSwap>(
                          new CreditDefaultSwap(Protection::Buyer, 100.0, 0.01,
                                                schedule, paymentConvention_,
                                                dayCounter_, settlesAccrual_,
                                                paysAtDefaultTime_));

        swap_->setPricingEngine(boost::shared_ptr<PricingEngine>(
                    new MidPointCdsEngine(Issuer(probability_, recoveryRate_),
                                          discountCurve_)));

    }

}


