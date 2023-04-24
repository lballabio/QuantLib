/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2016 Stefano Fondi

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

#include <ql/experimental/averageois/arithmeticoisratehelper.hpp>
#include <ql/experimental/averageois/makearithmeticaverageois.hpp>
#include <ql/pricingengines/swap/discountingswapengine.hpp>
#include <utility>

namespace QuantLib {

    namespace {
        void no_deletion(YieldTermStructure*) {}
    }

    ArithmeticOISRateHelper::ArithmeticOISRateHelper(Natural settlementDays,
                                                     const Period& tenor, // swap maturity
                                                     Frequency fixedLegPaymentFrequency,
                                                     const Handle<Quote>& fixedRate,
                                                     ext::shared_ptr<OvernightIndex> overnightIndex,
                                                     Frequency overnightLegPaymentFrequency,
                                                     Handle<Quote> spread,
                                                     Real meanReversionSpeed,
                                                     Real volatility,
                                                     bool byApprox,
                                                     Handle<YieldTermStructure> discount)
    : RelativeDateRateHelper(fixedRate), settlementDays_(settlementDays), tenor_(tenor),
      overnightIndex_(std::move(overnightIndex)), discountHandle_(std::move(discount)),
      fixedLegPaymentFrequency_(fixedLegPaymentFrequency),
      overnightLegPaymentFrequency_(overnightLegPaymentFrequency), spread_(std::move(spread)),
      mrs_(meanReversionSpeed), vol_(volatility), byApprox_(byApprox) {
        registerWith(overnightIndex_);
        registerWith(discountHandle_);
        registerWith(spread_);
        ArithmeticOISRateHelper::initializeDates();
    }

    void ArithmeticOISRateHelper::initializeDates() {

        // dummy OvernightIndex with curve/swap arguments
        // review here
        ext::shared_ptr<IborIndex> clonedIborIndex =
            overnightIndex_->clone(termStructureHandle_);
        ext::shared_ptr<OvernightIndex> clonedOvernightIndex =
            ext::dynamic_pointer_cast<OvernightIndex>(clonedIborIndex);

       swap_ = MakeArithmeticAverageOIS(tenor_, clonedOvernightIndex, 0.0)
                .withDiscountingTermStructure(discountRelinkableHandle_)
                .withSettlementDays(settlementDays_)
                .withFixedLegPaymentFrequency(fixedLegPaymentFrequency_)
                .withOvernightLegPaymentFrequency(overnightLegPaymentFrequency_)
                .withArithmeticAverage(mrs_, vol_, byApprox_);
        
        earliestDate_ = swap_->startDate();
        latestDate_ = swap_->maturityDate();
    }

    void ArithmeticOISRateHelper::setTermStructure(YieldTermStructure* t) {
        // do not set the relinkable handle as an observer -
        // force recalculation when needed
        bool observer = false;

        ext::shared_ptr<YieldTermStructure> temp(t, no_deletion);
        termStructureHandle_.linkTo(temp, observer);

        if (discountHandle_.empty())
            discountRelinkableHandle_.linkTo(temp, observer);
        else
            discountRelinkableHandle_.linkTo(*discountHandle_, observer);

        RelativeDateRateHelper::setTermStructure(t);
    }

    Real ArithmeticOISRateHelper::impliedQuote() const {
        QL_REQUIRE(termStructure_ != nullptr, "term structure not set");
        // we didn't register as observers - force calculation
        swap_->recalculate();
        //return swap_->fairRate();
        // weak implementation... to be improved
        static const Spread basisPoint = 1.0e-4;
        Real floatingLegNPV = swap_->overnightLegNPV();
        Spread spread = spread_.empty() ? 0.0 : spread_->value();
        Real spreadNPV = swap_->overnightLegBPS() / basisPoint*spread;
        Real totNPV = -(floatingLegNPV + spreadNPV);
        Real result = totNPV / (swap_->fixedLegBPS() / basisPoint);
        return result;
    }

    void ArithmeticOISRateHelper::accept(AcyclicVisitor& v) {
        auto* v1 = dynamic_cast<Visitor<ArithmeticOISRateHelper>*>(&v);
        if (v1 != nullptr)
            v1->visit(*this);
        else
            RateHelper::accept(v);
    }

}
