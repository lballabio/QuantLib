
/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file swaption.cpp
    \brief Swaption
*/

#include <ql/Instruments/swaption.hpp>
#include <ql/CashFlows/fixedratecoupon.hpp>
#include <ql/CashFlows/parcoupon.hpp>
#include <ql/Solvers1D/brent.hpp>

namespace QuantLib {

    namespace Instruments {

        using CashFlows::Coupon;
        using CashFlows::FixedRateCoupon;

        Swaption::Swaption(
            const Handle<SimpleSwap>& swap, const Exercise& exercise,
            const RelinkableHandle<TermStructure>& termStructure,
            const Handle<PricingEngine>& engine)
        : Option(engine), swap_(swap), exercise_(exercise),
          termStructure_(termStructure) {
            registerWith(swap_);
            registerWith(termStructure_);
        }

        bool Swaption::isExpired() const {
            return exercise_.dates().back() < termStructure_->referenceDate();
        }

        void Swaption::setupEngine() const {
            SwaptionArguments* arguments =
                dynamic_cast<SwaptionArguments*>(engine_->arguments());
                
            QL_REQUIRE(arguments != 0, "Swaption: "
                       "pricing engine does not supply needed arguments");

            Date settlement = termStructure_->referenceDate();
            DayCounter counter = termStructure_->dayCounter();
            Size i;

            arguments->payFixed = swap_->payFixedRate();
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

            arguments->nominal = swap_->nominal();

            const std::vector<Handle<CashFlow> >& fixedLeg = swap_->fixedLeg();

            arguments->fixedResetTimes.clear();
            arguments->fixedPayTimes.clear();
            arguments->fixedCoupons.clear();
            for (i=0; i<fixedLeg.size(); i++) {
                Handle<FixedRateCoupon> coupon = fixedLeg[i];

                Time time = counter.yearFraction(settlement, coupon->date());
                arguments->fixedPayTimes.push_back(time);
                time = counter.yearFraction(settlement, 
                                            coupon->accrualStartDate());
                arguments->fixedResetTimes.push_back(time);
                arguments->fixedCoupons.push_back(coupon->amount());
            }

            arguments->floatingResetTimes.clear();
            arguments->floatingPayTimes.clear();
            arguments->floatingAccrualTimes.clear();

            const std::vector<Handle<CashFlow> >& floatingLeg = 
                swap_->floatingLeg();
            std::vector<Handle<CashFlow> >::const_iterator begin, end;
            begin = floatingLeg.begin();
            end   = floatingLeg.end();

            for (; begin != end; ++begin) {
                Handle<Coupon> coupon = *begin;
                Date resetDate = coupon->accrualStartDate(); // already rolled
                Time time = counter.yearFraction(settlement, resetDate);
                arguments->floatingResetTimes.push_back(time);
                time = counter.yearFraction(settlement, coupon->date());
                arguments->floatingPayTimes.push_back(time);
                arguments->floatingAccrualTimes.push_back(
                                               coupon->accrualPeriod());
            }
            arguments->exerciseType = exercise_.type();
            arguments->exerciseTimes.clear();
            const std::vector<Date> dates = exercise_.dates();
            for (i=0; i<dates.size(); i++) {
                Time time = counter.yearFraction(settlement, dates[i]);
                arguments->exerciseTimes.push_back(time);
            }
        }

        void Swaption::performCalculations() const {
            Option::performCalculations();
            QL_ENSURE(NPV_ != Null<double>(),
                      "null value returned from swaption pricer");
        }

        void SwaptionArguments::validate() const {
            QL_REQUIRE(fixedPayTimes.size() == fixedCoupons.size(), 
                       "SwaptionArguments: Invalid pricing arguments");
        }
    
    }

}
