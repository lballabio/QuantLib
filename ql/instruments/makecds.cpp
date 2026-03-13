/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2014 Jose Aparicio
 Copyright (C) 2014 Peter Caspers

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

#include <ql/instruments/makecds.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/time/calendars/weekendsonly.hpp>


namespace QuantLib {

    MakeCreditDefaultSwap::MakeCreditDefaultSwap(const Period& tenor,
                                                 Rate runningSpread)
    : tenor_(tenor), runningSpread_(runningSpread) {}

    MakeCreditDefaultSwap::MakeCreditDefaultSwap(const Date& termDate,
                                                 Rate runningSpread)
    : termDate_(termDate), runningSpread_(runningSpread) {}

    MakeCreditDefaultSwap::MakeCreditDefaultSwap(const Schedule& schedule,
                                                 Rate runningSpread)
    : schedule_(schedule), runningSpread_(runningSpread) {}


    MakeCreditDefaultSwap::operator CreditDefaultSwap() const {
        ext::shared_ptr<CreditDefaultSwap> swap = *this;
        return *swap;
    }

    MakeCreditDefaultSwap::operator ext::shared_ptr<CreditDefaultSwap>() const {

        Date tradeDate = tradeDate_ != Date() ? tradeDate_ : Settings::instance().evaluationDate();
        Date upfrontDate = upfrontDate_ != Date() ? upfrontDate_ : WeekendsOnly().advance(tradeDate, cashSettlementDays_, Days);

        Date protectionStart = protectionStart_;
        if (protectionStart == Date()) {
            if (schedule_) { // NOLINT(readability-implicit-bool-conversion)
                protectionStart = schedule_->at(0);
            } else {
                if (rule_ == DateGeneration::CDS2015 || rule_ == DateGeneration::CDS) {
                    protectionStart = tradeDate;
                } else {
                    protectionStart = tradeDate + 1;
                }
            }
        }

        // schedule, tenor and term date come from different constructors; exactly one of them is not null.
        Schedule schedule;
        if (schedule_) { // NOLINT(readability-implicit-bool-conversion)
            schedule = *schedule_;
        } else {
            Date end;
            if (tenor_) { // NOLINT(readability-implicit-bool-conversion)
                if (rule_ == DateGeneration::CDS2015 || rule_ == DateGeneration::CDS || rule_ == DateGeneration::OldCDS) {
                    end = cdsMaturity(tradeDate, *tenor_, rule_);
                } else {
                    end = tradeDate + *tenor_;
                }
            } else {
                end = *termDate_; // NOLINT(bugprone-unchecked-optional-access)
            }

            schedule = Schedule(protectionStart, end, couponTenor_, WeekendsOnly(), convention_,
                                Unadjusted, rule_, false);
        }

        auto cds = ext::make_shared<CreditDefaultSwap>(
            side_, nominal_, upfrontRate_, runningSpread_, schedule, convention_,
            dayCounter_, settlesAccrual_, paysAtDefaultTime_, protectionStart, upfrontDate,
            claim_, lastPeriodDayCounter_, rebatesAccrual_, tradeDate, cashSettlementDays_);

        cds->setPricingEngine(engine_);
        return cds;
    }


    MakeCreditDefaultSwap &
    MakeCreditDefaultSwap::withSide(Protection::Side side) {
        side_ = side;
        return *this;
    }

    MakeCreditDefaultSwap &MakeCreditDefaultSwap::withNominal(Real nominal) {
        nominal_ = nominal;
        return *this;
    }

    MakeCreditDefaultSwap &
    MakeCreditDefaultSwap::withUpfrontRate(Real upfrontRate) {
        upfrontRate_ = upfrontRate;
        return *this;
    }

    MakeCreditDefaultSwap &
    MakeCreditDefaultSwap::withCouponTenor(Period couponTenor) {
        couponTenor_ = couponTenor;
        return *this;
    }

    MakeCreditDefaultSwap& MakeCreditDefaultSwap::withDateGenerationRule(DateGeneration::Rule rule) {
        rule_ = rule;
        return *this;
    }

    MakeCreditDefaultSwap& MakeCreditDefaultSwap::withConvention(BusinessDayConvention convention) {
        convention_ = convention;
        return *this;
    }

    MakeCreditDefaultSwap &
    MakeCreditDefaultSwap::withDayCounter(const DayCounter& dayCounter) {
        dayCounter_ = dayCounter;
        return *this;
    }

    MakeCreditDefaultSwap &
    MakeCreditDefaultSwap::settleAccrual(bool b) {
        settlesAccrual_ = b;
        return *this;
    }

    MakeCreditDefaultSwap &
    MakeCreditDefaultSwap::payAtDefaultTime(bool b) {
        paysAtDefaultTime_ = b;
        return *this;
    }

    MakeCreditDefaultSwap& MakeCreditDefaultSwap::withProtectionStart(Date d) {
        protectionStart_ = d;
        return *this;
    }

    MakeCreditDefaultSwap& MakeCreditDefaultSwap::withUpfrontDate(Date d) {
        upfrontDate_ = d;
        return *this;
    }

    MakeCreditDefaultSwap& MakeCreditDefaultSwap::withClaim(ext::shared_ptr<Claim> claim) {
        claim_ = claim;
        return *this;
    }

    MakeCreditDefaultSwap &MakeCreditDefaultSwap::withLastPeriodDayCounter(
        const DayCounter& lastPeriodDayCounter) {
        lastPeriodDayCounter_ = lastPeriodDayCounter;
        return *this;
    }

    MakeCreditDefaultSwap &
    MakeCreditDefaultSwap::rebateAccrual(bool b) {
        rebatesAccrual_ = b;
        return *this;
    }

    MakeCreditDefaultSwap& MakeCreditDefaultSwap::withTradeDate(Date tradeDate) {
        tradeDate_ = tradeDate;
        return *this;
    }

    MakeCreditDefaultSwap& MakeCreditDefaultSwap::withCashSettlementDays(Natural cashSettlementDays) {
        cashSettlementDays_ = cashSettlementDays;
        return *this;
    }

    MakeCreditDefaultSwap &MakeCreditDefaultSwap::withPricingEngine(
                               const ext::shared_ptr<PricingEngine> &engine) {
        engine_ = engine;
        return *this;
    }

}
