/*
 Copyright (C) 2001, 2002 Sadruddin Rejeb

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

    \fullpath
    ql/Instruments/%swaption.cpp
*/

// $Id$

#include <ql/Instruments/swaption.hpp>
#include <ql/CashFlows/floatingratecoupon.hpp>
#include <ql/Solvers1D/brent.hpp>

namespace QuantLib {

    namespace Instruments {

        using CashFlows::FloatingRateCoupon;

        Swaption::Swaption(
            const Handle<SimpleSwap>& swap, const Exercise& exercise,
            const RelinkableHandle<TermStructure>& termStructure,
            const Handle<PricingEngine>& engine)
        : Option(engine), swap_(swap), exercise_(exercise),
          termStructure_(termStructure) {
            registerWith(swap_);
            registerWith(termStructure_);
            registerWith(engine_);
        }

        void Swaption::setupEngine() const {
            SwaptionParameters* parameters =
                dynamic_cast<SwaptionParameters*>(
                    engine_->parameters());
            QL_REQUIRE(parameters != 0,
                       "pricing engine does not supply needed parameters");

            Date settlement = termStructure_->settlementDate();
            DayCounter counter = termStructure_->dayCounter();
            Size i;

            parameters->payFixed = swap_->payFixedRate();
            // volatilities are calculated for zero-spreaded swaps.
            // Therefore, the spread on the floating leg is removed
            // and a corresponding correction is made on the fixed leg.
            Spread correction = swap_->spread() * 
                                swap_->floatingLegBPS() /
                                swap_->fixedLegBPS();
            // the above is the opposite of the needed value since the 
            // two BPSs have opposite sign; hence the + sign below
            parameters->fixedRate = swap_->fixedRate() + correction;
            parameters->fairRate = swap_->fairRate() + correction;
            // this is passed explicitly for precision
            parameters->fixedBPS = QL_FABS(swap_->fixedLegBPS());

            parameters->nominal = swap_->nominal();

            const std::vector<Handle<CashFlow> >& fixedLeg = swap_->fixedLeg();

            parameters->fixedPayTimes.clear();
            parameters->fixedCoupons.clear();
            for (i=0; i<fixedLeg.size(); i++) {
                Time time = counter.yearFraction(settlement, 
                    fixedLeg[i]->date());
                parameters->fixedPayTimes.push_back(time);
                parameters->fixedCoupons.push_back(fixedLeg[i]->amount());
            }

            parameters->floatingResetTimes.clear();
            parameters->floatingPayTimes.clear();
            parameters->floatingAccrualTimes.clear();

            const std::vector<Handle<CashFlow> >& floatingLeg = 
                swap_->floatingLeg();
            std::vector<Handle<CashFlow> >::const_iterator begin, end;
            begin = floatingLeg.begin();
            end   = floatingLeg.end();

            for (; begin != end; ++begin) {
                Handle<FloatingRateCoupon> coupon = *begin;
                QL_ENSURE(!coupon.isNull(), "not a floating rate coupon");
                const Handle<Indexes::Xibor>& index = coupon->index();
/*                Date fixingDate = index->calendar().advance(
                    coupon->accrualStartDate(), -coupon->fixingDays(), Days,
                    index->rollingConvention());
                Date fixingValueDate = index->calendar().advance(
                    fixingDate, index->settlementDays(), Days,
                    index->rollingConvention());*/
                Date resetDate =  index->calendar().roll(
                    coupon->accrualStartDate(), index->rollingConvention());
                
                Time time = counter.yearFraction(settlement, resetDate);
                parameters->floatingResetTimes.push_back(time);
/*
                Date payDate = index->calendar().advance(
                    coupon->accrualEndDate(), 
                    index->settlementDays()-coupon->fixingDays(), Days,
                    index->rollingConvention());
*/
                time = counter.yearFraction(settlement, coupon->date());
                parameters->floatingPayTimes.push_back(time);
                parameters->floatingAccrualTimes.push_back(coupon->accrualPeriod());

            }
            parameters->exerciseType = exercise_.type();
            parameters->exerciseTimes.clear();
            const std::vector<Date> dates = exercise_.dates();
            for (i=0; i<dates.size(); i++) {
                Time time = counter.yearFraction(settlement, dates[i]);
                parameters->exerciseTimes.push_back(time);
            }
        }

        void Swaption::performCalculations() const {
            if (exercise_.dates().back() < termStructure_->settlementDate()) {
                isExpired_ = true;
                NPV_ = 0.0;
            } else {
                isExpired_ = false;
                Option::performCalculations();
            }
            QL_ENSURE(isExpired_ || NPV_ != Null<double>(),
                      "null value returned from cap/floor pricer");
        }

        void SwaptionParameters::validate() const {
            QL_REQUIRE(fixedPayTimes.size() == fixedCoupons.size(), 
                       "Invalid pricing parameters");
        }
    
    }

}
