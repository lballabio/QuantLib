
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

/*! \file caphelper.cpp
    \brief Cap calibration helper

    \fullpath
    ql/ShortRateModels/CalibrationHelpers/%caphelper.hpp
*/

// $Id$

#include "ql/ShortRateModels/CalibrationHelpers/caphelper.hpp"
#include "ql/CashFlows/cashflowvectors.hpp"
#include "ql/CashFlows/floatingratecoupon.hpp"
#include "ql/Instruments/swap.hpp"
#include "ql/Pricers/blackcapfloor.hpp"

namespace QuantLib {

    namespace ShortRateModels {

        namespace CalibrationHelpers {

            using CashFlows::FloatingRateCoupon;
            using CashFlows::FixedRateCouponVector;
            using CashFlows::FloatingRateCouponVector;
            using Instruments::VanillaCap;
            using Instruments::CapFloorArguments;
            using Instruments::Swap;
            using Indexes::Xibor;

            CapHelper::CapHelper(
                const Period& length,
                const RelinkableHandle<MarketElement>& volatility,
                const Handle<Xibor>& index,
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
                Date startDate = termStructure->referenceDate().
                    plus(indexTenor.length(), indexTenor.units());
                Date maturity = termStructure->referenceDate().
                    plus(length.length(), length.units());

                Handle<Xibor> dummyIndex(new Xibor("dummy",
                    indexTenor.length(),indexTenor.units(),
                    index->settlementDays(),
                    index->currency(),
                    index->calendar(),index->isAdjusted(),
                    index->rollingConvention(),
                    termStructure->dayCounter(),
                    termStructure));

                std::vector<double> nominals(1,1.0);
                std::vector<Handle<CashFlow> > floatingLeg = 
                    FloatingRateCouponVector(nominals, startDate, maturity, 
                        frequency, index->calendar(), 
                        index->rollingConvention(), index, 0, 
                        std::vector<double>(1, 0.0));
                std::vector<Handle<CashFlow> > fixedLeg = 
                    FixedRateCouponVector(
                        nominals, std::vector<Rate>(1, fixedRate), 
                        startDate, maturity, frequency, index->calendar(), 
                        index->rollingConvention(), false, 
                        index->dayCounter(), index->dayCounter());

                Handle<Swap> swap(
                    new Swap(floatingLeg, fixedLeg, termStructure));
                Rate fairRate = fixedRate - 
                    swap->NPV()/swap->secondLegBPS();
                engine_  = Handle<PricingEngine>( 
                    new Pricers::BlackCapFloor(blackModel_));
                cap_ = Handle<VanillaCap>(
                    new VanillaCap(floatingLeg, 
                                   std::vector<Rate>(1, fairRate), 
                                   termStructure, engine_));
                marketValue_ = blackPrice(volatility_->value());
            }

            void CapHelper::addTimes(std::list<Time>& times) const {
                CapFloorArguments* params =
                    dynamic_cast<CapFloorArguments*>(engine_->arguments());
                Size nPeriods = params->startTimes.size();
                for (Size i=0; i<nPeriods; i++) {
                    times.push_back(params->startTimes[i]);
                    times.push_back(params->endTimes[i]);
                }
            }

            double CapHelper::modelValue() {
                cap_->setPricingEngine(engine_);
                return cap_->NPV();
            }

            double CapHelper::blackPrice(double sigma) const {
                Handle<MarketElement> vol(new SimpleMarketElement(sigma));
                Handle<BlackModel> blackModel(
                    new BlackModel(RelinkableHandle<MarketElement>(vol), 
                                   termStructure_));
                Handle<PricingEngine> black(
                    new Pricers::BlackCapFloor(blackModel));
                cap_->setPricingEngine(black);
                double value = cap_->NPV();
                cap_->setPricingEngine(engine_);
                return value;
            }

        }

    }

}
