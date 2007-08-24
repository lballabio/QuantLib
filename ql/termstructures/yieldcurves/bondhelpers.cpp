/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005 Toyin Akin

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

#include <ql/termstructures/yieldcurves/bondhelpers.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/pricingengines/bond/discountingbondengine.hpp>
#include <ql/settings.hpp>

namespace QuantLib {

    namespace {
        void no_deletion(YieldTermStructure*) {}
    }

    FixedCouponBondHelper::FixedCouponBondHelper(
                                    const Handle<Quote>& cleanPrice,
                                    Natural settlementDays,
                                    const Schedule& schedule,
                                    const std::vector<Rate>& coupons,
                                    const DayCounter& paymentDayCounter,
                                    BusinessDayConvention paymentConvention,
                                    Real redemption,
                                    const Date& issueDate)
    : RateHelper(cleanPrice), settlementDays_(settlementDays),
      schedule_(schedule), coupons_(coupons),
      paymentDayCounter_(paymentDayCounter),
      paymentConvention_(paymentConvention),
      redemption_(redemption), issueDate_(issueDate) {

        latestDate_ = schedule.endDate();
        registerWith(Settings::instance().evaluationDate());
    }

    void FixedCouponBondHelper::setTermStructure(YieldTermStructure* t) {
        // do not set the relinkable handle as an observer -
        // force recalculation when needed
        termStructureHandle_.linkTo(
                 boost::shared_ptr<YieldTermStructure>(t,no_deletion), false);

        RateHelper::setTermStructure(t);

        bond_ = boost::shared_ptr<FixedRateBond>(new
            FixedRateBond(settlementDays_, 100.0, schedule_,
                            coupons_, paymentDayCounter_, paymentConvention_,
                            redemption_, issueDate_));
        boost::shared_ptr<PricingEngine> bondEngine(
                             new DiscountingBondEngine(termStructureHandle_));
        bond_->setPricingEngine(bondEngine);
    }

    Real FixedCouponBondHelper::impliedQuote() const {
        QL_REQUIRE(termStructure_ != 0, "term structure not set");
        // we didn't register as observers - force calculation
        bond_->recalculate();
        return bond_->cleanPrice();
    }

}

