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
#include "ql/Math/normaldistribution.hpp"
#include "ql/Pricers/analyticalcapfloor.hpp"
#include "ql/Pricers/treecapfloor.hpp"
#include "ql/Solvers1D/brent.hpp"

#include <iostream>
using std::cout;
using std::endl;

namespace QuantLib {

    namespace InterestRateModelling {

        namespace CalibrationHelpers {

            using CashFlows::FloatingRateCoupon;
            using CashFlows::FixedRateCouponVector;
            using CashFlows::FloatingRateCouponVector;
            using Instruments::VanillaCap;
            using Instruments::CapFloorParameters;
            using Instruments::SimpleSwap;
            using Instruments::Swap;
            using Pricers::CapFloorPricingEngine;

            CapHelper::CapHelper(
                const Period& tenor,
                const RelinkableHandle<MarketElement>& volatility,
                const Handle<Indexes::Xibor>& index,
                const RelinkableHandle<TermStructure>& termStructure)
            : CalibrationHelper(volatility), termStructure_(termStructure) {

                Period indexTenor = index->tenor();
                int frequency;
                if (indexTenor.units() == Months)
                    frequency = 12/indexTenor.length();
                else if (indexTenor.units() == Years)
                    frequency = 1/indexTenor.length();
                else
                    throw Error("index tenor not valid!");
                Rate fixedRate = 0.04;//dummy value
                Date startDate = termStructure->settlementDate().
                    plus(indexTenor.length(), indexTenor.units());
                Date maturity = termStructure->settlementDate().
                    plus(tenor.length(), tenor.units());

                std::vector<Handle<CashFlow> > fixedLeg, floatingLeg;
                std::vector<double> nominals(1,1.0);

                floatingLeg = FloatingRateCouponVector(
                    nominals, startDate, maturity, frequency,
                    index->calendar(), index->rollingConvention(), 
                    termStructure, index, 0, std::vector<double>(1, 0.0));
                fixedLeg = FixedRateCouponVector(
                    nominals, std::vector<Rate>(1, fixedRate), 
                    startDate, maturity, frequency, 
                    index->calendar(), index->rollingConvention(),
                    false, index->dayCounter(), index->dayCounter());

                SimpleSwap swap(false, fixedLeg, floatingLeg, termStructure);
                Rate fairFixedRate = fixedRate - swap.NPV()/swap.fixedLegBPS();
                fixedLeg = FixedRateCouponVector(
                    nominals, std::vector<Rate>(1, fairFixedRate), 
                    startDate, maturity, frequency, 
                    index->calendar(), index->rollingConvention(),
                    false, index->dayCounter(), index->dayCounter());

                swap_ = Handle<SimpleSwap>(new 
                    SimpleSwap(false, fixedLeg, floatingLeg, termStructure));
                engine_ = Handle<CapFloorPricingEngine>(
                    new Pricers::TreeCapFloor(10));
                cap_ = Handle<VanillaCap>(
                    new VanillaCap( swap_, std::vector<Rate>(1, fairFixedRate),
                    termStructure, engine_));
                cap_->setPricingEngine(engine_);

                marketValue_ = blackPrice(volatility_->value());
            }

            double CapHelper::modelValue(const Handle<Model>& model) {
                if (model->hasDiscountBondOptionFormula())
                    engine_ = Handle<CapFloorPricingEngine>(
                        new Pricers::AnalyticalCapFloor());
                else
                    engine_ = Handle<CapFloorPricingEngine>(
                        new Pricers::TreeCapFloor(10));
                engine_->setModel(model);
                cap_->setPricingEngine(engine_);
                cap_->recalculate();
                return cap_->NPV();
            }

            double CapHelper::blackPrice(double sigma) const {
                Math::CumulativeNormalDistribution f;
                double value = 0.0;

                CapFloorParameters* params =
                    dynamic_cast<CapFloorParameters*>(engine_->parameters());
                for (Size i=0; i<params->startTimes.size(); i++) {
                    Rate exerciseRate;
                    if (i<params->exerciseRates.size())
                        exerciseRate = params->exerciseRates[i];
                    else
                        exerciseRate = params->exerciseRates.back();

                    Time start = params->startTimes[i];
                    Time end = params->endTimes[i];
                    double tenor = end - start;
                    double p = termStructure_->discount(start);
                    double q = termStructure_->discount(end);
                    double forward = QL_LOG(p/q)/tenor;
                    double capletValue;
                    if (start > QL_EPSILON) {
                        double v = sigma*QL_SQRT(start);
                        double d1 = QL_LOG(forward/exerciseRate)/v+ 0.5*v;
                        double d2 = d1 - v;
                        capletValue = q*tenor*
                            (forward*f(d1) - exerciseRate*f(d2));
                    } else {
                        capletValue = q*tenor*
                            QL_MAX(forward - exerciseRate, 0.0);
                    }
                    value += capletValue;
                }
                return value;
            }
        }
    }
}
