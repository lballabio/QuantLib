/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Ferdinando Ametrano
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

namespace QuantLib {

    namespace {
        void no_deletion(YieldTermStructure*) {}
    }

    FixedRateBondHelper::FixedRateBondHelper(
                                    const Handle<Quote>& cleanPrice,
                                    Natural settlementDays,
                                    Real faceAmount,
                                    const Schedule& schedule,
                                    const std::vector<Rate>& coupons,
                                    const DayCounter& dayCounter,
                                    BusinessDayConvention paymentConvention,
                                    Real redemption,
                                    const Date& issueDate)
    : BootstrapHelper<YieldTermStructure>(cleanPrice) {

        bond_ = boost::shared_ptr<FixedRateBond>(new
            FixedRateBond(settlementDays, faceAmount, schedule,
                          coupons, dayCounter, paymentConvention,
                          redemption, issueDate));

        latestDate_ = bond_->maturityDate();
        registerWith(Settings::instance().evaluationDate());

        boost::shared_ptr<PricingEngine> bondEngine(new
            DiscountingBondEngine(termStructureHandle_));
        bond_->setPricingEngine(bondEngine);
    }

    FixedRateBondHelper::FixedRateBondHelper(
                                const Handle<Quote>& cleanPrice,
                                const boost::shared_ptr<FixedRateBond>& bond)
    : BootstrapHelper<YieldTermStructure>(cleanPrice) {

        latestDate_ = bond_->maturityDate();
        registerWith(Settings::instance().evaluationDate());

        boost::shared_ptr<PricingEngine> bondEngine(new
            DiscountingBondEngine(termStructureHandle_));
        bond_->setPricingEngine(bondEngine);
    }

    void FixedRateBondHelper::setTermStructure(YieldTermStructure* t) {
        // do not set the relinkable handle as an observer -
        // force recalculation when needed
        termStructureHandle_.linkTo(
                 boost::shared_ptr<YieldTermStructure>(t,no_deletion), false);

        BootstrapHelper<YieldTermStructure>::setTermStructure(t);
    }

    boost::shared_ptr<FixedRateBond> FixedRateBondHelper::bond() const {
        return bond_;
    }

    Real FixedRateBondHelper::impliedQuote() const {
        QL_REQUIRE(termStructure_ != 0, "term structure not set");
        // we didn't register as observers - force calculation
        bond_->recalculate();
        return bond_->cleanPrice();
    }

}
