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
/*! \file caphelper.cpp
    \brief Cap calibration helper

    \fullpath
    ql/InterestRateModelling/CalibrationHelpers/%caphelper.hpp
*/

// $Id$

#include "ql/InterestRateModelling/CalibrationHelpers/caphelper.hpp"
#include "ql/CashFlows/cashflowvectors.hpp"
#include "ql/CashFlows/floatingratecoupon.hpp"
#include "ql/Instruments/swap.hpp"
#include "ql/Pricers/blackcapfloor.hpp"

namespace QuantLib {

    namespace InterestRateModelling {

        namespace CalibrationHelpers {

            using CashFlows::FloatingRateCoupon;
            using CashFlows::FixedRateCouponVector;
            using CashFlows::FloatingRateCouponVector;
            using Instruments::VanillaCap;
            using Instruments::CapFloorParameters;
            using Instruments::Swap;

            CapHelper::CapHelper(
                const Period& length,
                const RelinkableHandle<MarketElement>& volatility,
                const Handle<Indexes::Xibor>& index,
                const RelinkableHandle<TermStructure>& termStructure)
            : CalibrationHelper(volatility), termStructure_(termStructure) {

                Period indexTenor = index->tenor();
                int frequency;
                if (indexTenor.units() == Months) {
                    QL_REQUIRE((12%indexTenor.length()) == 0, 
                        "Invalid index tenor");
                    frequency = 12/indexTenor.length();
                } else if (indexTenor.units() == Years) {
                    QL_REQUIRE(indexTenor.length()==1, "Invalid index tenor");
                    frequency=1;
                } else
                    throw Error("Invalid index tenor");
                Rate fixedRate = 0.04;//dummy value
                Date startDate = termStructure->settlementDate().
                    plus(indexTenor.length(), indexTenor.units());
                Date maturity = termStructure->settlementDate().
                    plus(length.length(), length.units());

                std::vector<double> nominals(1,1.0);
                FloatingRateCouponVector floatingLeg(
                    nominals, startDate, maturity, frequency,
                    index->calendar(), index->rollingConvention(), 
                    termStructure, index, 0, std::vector<double>(1, 0.0));
                FixedRateCouponVector fixedLeg(
                    nominals, std::vector<Rate>(1, fixedRate), 
                    startDate, maturity, frequency, 
                    index->calendar(), index->rollingConvention(),
                    false, index->dayCounter(), index->dayCounter());

                Handle<Swap> swap(
                    new Swap(floatingLeg, fixedLeg, termStructure));
                Rate fairRate = fixedRate - 
                    swap->NPV()/swap->secondLegBPS();
                engine_  = Handle<OptionPricingEngine>( 
                    new Pricers::BlackCapFloor(blackModel_));
                cap_ = Handle<VanillaCap>(
                    new VanillaCap(floatingLeg, std::vector<Rate>(1, fairRate), 
                                   termStructure, engine_));
                marketValue_ = blackPrice(volatility_->value());
            }

            void CapHelper::addTimes(std::list<Time>& times) const {
                CapFloorParameters* params =
                    dynamic_cast<CapFloorParameters*>(engine_->parameters());
                Size nPeriods = params->startTimes.size();
                for (Size i=0; i<nPeriods; i++) {
                    times.push_back(params->startTimes[i]);
                    times.push_back(params->endTimes[i]);
                }
            }

            void CapHelper::setModel(const Handle<Model>& model) {
                Handle<Pricers::CapFloorPricingEngine<Model> > engine(engine_);
                engine->setModel(model);
            }

            double CapHelper::modelValue() {
                cap_->setPricingEngine(engine_);
                return cap_->NPV();
            }

            double CapHelper::blackPrice(double sigma) const {
                Handle<OptionPricingEngine> black(
                    new Pricers::BlackCapFloor(blackModel_));
                cap_->setPricingEngine(black);
                double value = cap_->NPV();
                cap_->setPricingEngine(engine_);
                return value;
            }
        }
    }
}
