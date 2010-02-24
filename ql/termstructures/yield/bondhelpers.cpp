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

namespace QuantLib {

    namespace {
        void no_deletion(YieldTermStructure*) {}
    }

    BondHelper::BondHelper(const Handle<Quote>& cleanPrice,
                           const boost::shared_ptr<Bond>& bond)
    : RateHelper(cleanPrice), bond_(new Bond(*bond)) {

        // the bond's last cashflow date, which can be later than
        // bond's maturity date because of adjustment
        latestDate_ = bond_->cashflows().back()->date();
        earliestDate_ = bond_->nextCashFlowDate();

        boost::shared_ptr<PricingEngine> bondEngine(new
            DiscountingBondEngine(termStructureHandle_));
        bond_->setPricingEngine(bondEngine);
    }

    void BondHelper::setTermStructure(YieldTermStructure* t) {
        // do not set the relinkable handle as an observer -
        // force recalculation when needed
        termStructureHandle_.linkTo(
                 boost::shared_ptr<YieldTermStructure>(t,no_deletion), false);

        BootstrapHelper<YieldTermStructure>::setTermStructure(t);
    }

    Real BondHelper::impliedQuote() const {
        QL_REQUIRE(termStructure_ != 0, "term structure not set");
        // we didn't register as observers - force calculation
        bond_->recalculate();
        return bond_->cleanPrice();
    }

    void BondHelper::accept(AcyclicVisitor& v) {
        Visitor<BondHelper>* v1 =
            dynamic_cast<Visitor<BondHelper>*>(&v);
        if (v1 != 0)
            v1->visit(*this);
        else
            BootstrapHelper<YieldTermStructure>::accept(v);
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
    : BondHelper(cleanPrice, boost::shared_ptr<Bond>(new
        FixedRateBond(settlementDays, faceAmount, schedule,
                      coupons, dayCounter, paymentConvention,
                      redemption, issueDate))) {
        fixedRateBond_ = boost::shared_ptr<FixedRateBond>(new
            FixedRateBond(settlementDays, faceAmount, schedule,
                          coupons, dayCounter, paymentConvention,
                          redemption, issueDate));
    }

    void FixedRateBondHelper::accept(AcyclicVisitor& v) {
        Visitor<FixedRateBondHelper>* v1 =
            dynamic_cast<Visitor<FixedRateBondHelper>*>(&v);
        if (v1 != 0)
            v1->visit(*this);
        else
            BootstrapHelper<YieldTermStructure>::accept(v);
    }

}
