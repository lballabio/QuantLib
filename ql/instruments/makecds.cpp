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
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/instruments/makecds.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/time/calendars/weekendsonly.hpp>


namespace QuantLib {

    MakeCreditDefaultSwap::MakeCreditDefaultSwap(const Period &tenor,
                                                 const Real couponRate)
        : side_(Protection::Buyer), nominal_(1.0), tenor_(tenor),
          couponTenor_(3 * Months), couponRate_(couponRate), upfrontRate_(0.0),
          dayCounter_(Actual360()), lastPeriodDayCounter_(Actual360(true)),
          rule_(DateGeneration::CDS), cashSettlementDays_(3) {}

    MakeCreditDefaultSwap::MakeCreditDefaultSwap(const Date &termDate,
                                                 const Real couponRate)
        : side_(Protection::Buyer), nominal_(1.0), termDate_(termDate),
          couponTenor_(3 * Months), couponRate_(couponRate), upfrontRate_(0.0),
          dayCounter_(Actual360()), lastPeriodDayCounter_(Actual360(true)),
          rule_(DateGeneration::CDS), cashSettlementDays_(3) {}

    MakeCreditDefaultSwap::operator CreditDefaultSwap() const {
        std::shared_ptr<CreditDefaultSwap> swap = *this;
        return *swap;
    }

    MakeCreditDefaultSwap::operator std::shared_ptr<CreditDefaultSwap>() const {

        Date tradeDate = (tradeDate_ != Null<Date>()) ? tradeDate_ : Settings::instance().evaluationDate();
        Date upfrontDate = WeekendsOnly().advance(tradeDate, cashSettlementDays_, Days);

        Date protectionStart;
        if (rule_ == DateGeneration::CDS2015 || rule_ == DateGeneration::CDS) {
            protectionStart = tradeDate;
        } else {
            protectionStart = tradeDate + 1;
        }

        Date end;
        if (tenor_) { // NOLINT(readability-implicit-bool-conversion)
            if (rule_ == DateGeneration::CDS2015 || rule_ == DateGeneration::CDS || rule_ == DateGeneration::OldCDS) {
                end = cdsMaturity(tradeDate, *tenor_, rule_);
            } else {
                end = tradeDate + *tenor_;
            }
        } else {
            end = *termDate_;
        }

        Schedule schedule(protectionStart, end, couponTenor_, WeekendsOnly(), Following,
                          Unadjusted, rule_, false);

        std::shared_ptr<CreditDefaultSwap> cds =
            std::shared_ptr<CreditDefaultSwap>(new CreditDefaultSwap(
                side_, nominal_, upfrontRate_, couponRate_, schedule, Following,
                dayCounter_, true, true, protectionStart, upfrontDate,
                std::shared_ptr<Claim>(), lastPeriodDayCounter_, true, tradeDate, cashSettlementDays_));

        cds->setPricingEngine(engine_);
        return cds;

    }

    MakeCreditDefaultSwap &
    MakeCreditDefaultSwap::withUpfrontRate(Real upfrontRate) {
        upfrontRate_ = upfrontRate;
        return *this;
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
    MakeCreditDefaultSwap::withCouponTenor(Period couponTenor) {
        couponTenor_ = couponTenor;
        return *this;
    }

    MakeCreditDefaultSwap &
    MakeCreditDefaultSwap::withDayCounter(DayCounter &dayCounter) {
        dayCounter_ = dayCounter;
        return *this;
    }

    MakeCreditDefaultSwap &MakeCreditDefaultSwap::withLastPeriodDayCounter(
        DayCounter &lastPeriodDayCounter) {
        lastPeriodDayCounter_ = lastPeriodDayCounter;
        return *this;
    }

    MakeCreditDefaultSwap& MakeCreditDefaultSwap::withDateGenerationRule(DateGeneration::Rule rule) {
        rule_ = rule;
        return *this;
    }

    MakeCreditDefaultSwap& MakeCreditDefaultSwap::withCashSettlementDays(Natural cashSettlementDays) {
        cashSettlementDays_ = cashSettlementDays;
        return *this;
    }

    MakeCreditDefaultSwap &MakeCreditDefaultSwap::withPricingEngine(
        const std::shared_ptr<PricingEngine> &engine) {
        engine_ = engine;
        return *this;
    }

    MakeCreditDefaultSwap& MakeCreditDefaultSwap::withTradeDate(const Date& tradeDate) {
        tradeDate_ = tradeDate;
        return *this;
    }

}
