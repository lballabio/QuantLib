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
            const Handle<OptionPricingEngine>& engine)
        : Option(engine), swap_(swap), exercise_(exercise),
          termStructure_(termStructure) {}

        Swaption::~Swaption() {}

        void Swaption::setupEngine() const {
            SwaptionParameters* parameters =
                dynamic_cast<SwaptionParameters*>(
                    engine_->parameters());
            QL_REQUIRE(parameters != 0,
                       "pricing engine does not supply needed parameters");

            Date settlement = termStructure_->settlementDate();
            DayCounter counter = termStructure_->dayCounter();
            Size i;

            const std::vector<Handle<CashFlow> >& fixedLeg = swap_->fixedLeg();
            parameters->payFixed = swap_->payFixedRate();

            Handle<SimpleSwap> vanilla(swap_);
            if (!vanilla.isNull()) {
                parameters->isVanilla = true;
                parameters->fixedRate = vanilla->fixedRate();
                parameters->fairRate = vanilla->fairRate();
            } else {
                parameters->isVanilla = false;
            }
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
            parameters->nominals.clear();
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
                parameters->nominals.push_back(coupon->nominal());
            }
            parameters->exerciseType = exercise_.type();
            parameters->exerciseTimes.clear();
            const std::vector<Date> dates = exercise_.dates();
            for (i=0; i<dates.size(); i++) {
                Date exerciseDate = exercise_.calendar().advance(
                    dates[i], exercise_.settlementDays(), Days,
                    exercise_.rollingConvention());
                Time time = counter.yearFraction(settlement, exerciseDate);
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
    
    }

}
