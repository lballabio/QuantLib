/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008, 2009 Ferdinando Ametrano
 Copyright (C) 2005 Toyin Akin
 Copyright (C) 2007 StatPro Italia srl

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

#include <ql/termstructures/yield/bondhelpers.hpp>
#include <ql/pricingengines/bond/discountingbondengine.hpp>
#include <ql/time/schedule.hpp>
#include <ql/settings.hpp>
#include <ql/utilities/null_deleter.hpp>

namespace QuantLib {

    BondHelper::BondHelper(const Handle<Quote>& price,
                           const std::shared_ptr<Bond>& bond,
                           const Bond::Price::Type priceType)
    : RateHelper(price), bond_(std::make_shared<Bond>(*bond)), priceType_(priceType) {

        // the bond's last cashflow date, which can be later than
        // bond's maturity date because of adjustment
        latestDate_ = bond_->cashflows().back()->date();
        earliestDate_ = bond_->nextCashFlowDate();

        bond_->setPricingEngine(
             std::make_shared<DiscountingBondEngine>(termStructureHandle_));
    }

    void BondHelper::setTermStructure(YieldTermStructure* t) {
        // do not set the relinkable handle as an observer -
        // force recalculation when needed
        termStructureHandle_.linkTo(
            std::shared_ptr<YieldTermStructure>(t, null_deleter()), false);

        BootstrapHelper<YieldTermStructure>::setTermStructure(t);
    }

    Real BondHelper::impliedQuote() const {
        QL_REQUIRE(termStructure_ != nullptr, "term structure not set");
        // we didn't register as observers - force calculation
        bond_->recalculate();

        switch (priceType_) {
            case Bond::Price::Clean:
                return bond_->cleanPrice();
                break;

            case Bond::Price::Dirty:
                return bond_->dirtyPrice();
                break;

            default:
                QL_FAIL("This price type isn't implemented.");
        }
    }

    void BondHelper::accept(AcyclicVisitor& v) {
        auto* v1 = dynamic_cast<Visitor<BondHelper>*>(&v);
        if (v1 != nullptr)
            v1->visit(*this);
        else
            BootstrapHelper<YieldTermStructure>::accept(v);
    }

    FixedRateBondHelper::FixedRateBondHelper(
                                    const Handle<Quote>& price,
                                    Natural settlementDays,
                                    Real faceAmount,
                                    const Schedule& schedule,
                                    const std::vector<Rate>& coupons,
                                    const DayCounter& dayCounter,
                                    BusinessDayConvention paymentConvention,
                                    Real redemption,
                                    const Date& issueDate,
                                    const Calendar& paymentCalendar,
                                    const Period& exCouponPeriod,
                                    const Calendar& exCouponCalendar,
                                    const BusinessDayConvention exCouponConvention,
                                    bool exCouponEndOfMonth,
                                    const Bond::Price::Type priceType)
    : BondHelper(price,
                 std::shared_ptr<Bond>(
                     new FixedRateBond(settlementDays, faceAmount, schedule,
                                       coupons, dayCounter, paymentConvention,
                                       redemption, issueDate, paymentCalendar,
                                       exCouponPeriod, exCouponCalendar,
                                       exCouponConvention, exCouponEndOfMonth)),
                 priceType) {
        fixedRateBond_ = std::dynamic_pointer_cast<FixedRateBond>(bond_);
    }

    void FixedRateBondHelper::accept(AcyclicVisitor& v) {
        auto* v1 = dynamic_cast<Visitor<FixedRateBondHelper>*>(&v);
        if (v1 != nullptr)
            v1->visit(*this);
        else
            BondHelper::accept(v);
    }

    CPIBondHelper::CPIBondHelper(
                            const Handle<Quote>& price,
                            Natural settlementDays,
                            Real faceAmount,
                            const bool growthOnly,
                            Real baseCPI,
                            const Period& observationLag,
                            const std::shared_ptr<ZeroInflationIndex>& cpiIndex,
                            CPI::InterpolationType observationInterpolation,
                            const Schedule& schedule,
                            const std::vector<Rate>& fixedRate,
                            const DayCounter& accrualDayCounter,
                            BusinessDayConvention paymentConvention,
                            const Date& issueDate,
                            const Calendar& paymentCalendar,
                            const Period& exCouponPeriod,
                            const Calendar& exCouponCalendar,
                            const BusinessDayConvention exCouponConvention,
                            bool exCouponEndOfMonth,
                            const Bond::Price::Type priceType)
    : BondHelper(price,
                 std::shared_ptr<Bond>(
                     new CPIBond(settlementDays, faceAmount, growthOnly, baseCPI,
                                       observationLag, cpiIndex, observationInterpolation,
                                       schedule, fixedRate, accrualDayCounter, paymentConvention,
                                       issueDate, paymentCalendar, exCouponPeriod, exCouponCalendar,
                                       exCouponConvention, exCouponEndOfMonth)),
                 priceType) {
        cpiBond_ = std::dynamic_pointer_cast<CPIBond>(bond_);
    }

    void CPIBondHelper::accept(AcyclicVisitor& v) {
        auto* v1 = dynamic_cast<Visitor<CPIBondHelper>*>(&v);
        if (v1 != nullptr)
            v1->visit(*this);
        else
            BondHelper::accept(v);
    }

}
