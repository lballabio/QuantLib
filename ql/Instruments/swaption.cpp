
/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/Instruments/swaption.hpp>
#include <ql/CashFlows/fixedratecoupon.hpp>
#include <ql/CashFlows/parcoupon.hpp>
#include <ql/Solvers1D/brent.hpp>

namespace QuantLib {

    Swaption::Swaption(const boost::shared_ptr<SimpleSwap>& swap,
                       const boost::shared_ptr<Exercise>& exercise,
                       const Handle<YieldTermStructure>& termStructure,
                       const boost::shared_ptr<PricingEngine>& engine)
    : Option(boost::shared_ptr<Payoff>(), exercise, engine), swap_(swap),
      termStructure_(termStructure) {
        registerWith(swap_);
        registerWith(termStructure_);
    }

    bool Swaption::isExpired() const {
        return exercise_->dates().back() < termStructure_->referenceDate();
    }

    void Swaption::setupArguments(Arguments* args) const {

        swap_->setupArguments(args);

        Swaption::arguments* arguments =
            dynamic_cast<Swaption::arguments*>(args);

        QL_REQUIRE(arguments != 0, "wrong argument type");

        Date settlement = termStructure_->referenceDate();
        /// ??? ///
        #ifndef QL_DISABLE_DEPRECATED
        DayCounter counter = termStructure_->dayCounter();
        #else
        DayCounter counter = Settings::instance().dayCounter();
        #endif

        // volatilities are calculated for zero-spreaded swaps.
        // Therefore, the spread on the floating leg is removed
        // and a corresponding correction is made on the fixed leg.
        Spread correction = swap_->spread() *
            swap_->floatingLegBPS() /
            swap_->fixedLegBPS();
        // the above is the opposite of the needed value since the
        // two BPSs have opposite sign; hence the + sign below
        arguments->fixedRate = swap_->fixedRate() + correction;
        arguments->fairRate = swap_->fairRate() + correction;
        // this is passed explicitly for precision
        arguments->fixedBPS = QL_FABS(swap_->fixedLegBPS());

        arguments->exercise = exercise_;
        arguments->stoppingTimes.clear();
//        const std::vector<Date> dates = exercise_->dates();
        for (Size i=0; i<exercise_->dates().size(); i++) {
            Time time = counter.yearFraction(settlement,
                exercise_->dates()[i]);
            arguments->stoppingTimes.push_back(time);
        }
    }

    void Swaption::arguments::validate() const {
        #if defined(QL_PATCH_MSVC6)
        SimpleSwap::arguments copy = *this;
        copy.validate();
        #else
        SimpleSwap::arguments::validate();
        #endif

        QL_REQUIRE(fixedRate != Null<Real>(),
                   "fixed swap rate null or not set");
        QL_REQUIRE(fairRate != Null<Real>(),
                   "fair swap rate null or not set");
        QL_REQUIRE(fixedBPS != Null<Real>(),
                   "fixed swap BPS null or not set");
    }

}
