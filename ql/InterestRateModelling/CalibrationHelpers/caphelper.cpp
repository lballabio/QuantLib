
/*
 Copyright (C) 2000, 2001, 2002 Sadruddin Rejeb

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software developed by the QuantLib Group; you can
 redistribute it and/or modify it under the terms of the QuantLib License;
 either version 1.0, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 QuantLib License for more details.

 You should have received a copy of the QuantLib License along with this
 program; if not, please email ferdinando@ametrano.net

 The QuantLib License is also available at http://quantlib.org/license.html
 The members of the QuantLib Group are listed in the QuantLib License
*/
/*! \file caphelper.cpp
    \brief Cap calibration helper

    \fullpath
    ql/InterestRateModelling/CalibrationHelpers/%caphelper.hpp
*/

// $Id$

#include "ql/InterestRateModelling/CalibrationHelpers/caphelper.hpp"
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
            using Instruments::VanillaCap;
            using Instruments::CapFloorParameters;
            using Instruments::SimpleSwap;
            using Pricers::CapFloorPricingEngine;

            CapHelper::CapHelper(
                const Period& tenor,
                const Handle<Indexes::Xibor>& index,
                const RelinkableHandle<TermStructure>& termStructure)
            : termStructure_(termStructure) {

                Period indexTenor = index->tenor();
                int frequency;
                if (indexTenor.units() == Months)
                    frequency = 12/indexTenor.length();
                else if (indexTenor.units() == Years)
                    frequency = 1/indexTenor.length();
                else
                    throw Error("index tenor not valid!");
                Rate fixedRate = 0.04;//dummy value
                SimpleSwap swap(
                  false,
                  termStructure->settlementDate(),
                  tenor.length(),
                  tenor.units(),
                  index->calendar(),
                  index->rollingConvention(),
                  std::vector<double>(1, 1.0),
                  frequency,
                  std::vector<double>(1, fixedRate),
                  false,
                  index->dayCounter(),
                  frequency,
                  index,
                  0,//FIXME
                  std::vector<double>(1, 0.0),
                  termStructure);
                Rate fairFixedRate = fixedRate - swap.NPV()/swap.fixedLegBPS();
                swap_ = Handle<SimpleSwap>(new SimpleSwap(
                  false,
                  termStructure->settlementDate(),
                  tenor.length(),
                  tenor.units(),
                  index->calendar(),
                  index->rollingConvention(),
                  std::vector<double>(1, 1.0),
                  frequency,
                  std::vector<double>(1, fairFixedRate),
                  false,
                  index->dayCounter(),
                  frequency,
                  index,
                  0,//FIXME
                  std::vector<double>(1, 0.0),
                  termStructure));
                engine_ = Handle<CapFloorPricingEngine>(
                    new Pricers::TreeCapFloor(100));
                cap_ = Handle<VanillaCap>(
                    new VanillaCap( *swap_, std::vector<Rate>(1, fairFixedRate),
                    termStructure, engine_));
                cap_->setPricingEngine(engine_);
            }

            double CapHelper::modelValue(const Handle<Model>& model) {
                if (model->hasDiscountBondOptionFormula())
                    engine_ = Handle<CapFloorPricingEngine>(
                        new Pricers::AnalyticalCapFloor());
                else
                    engine_ = Handle<CapFloorPricingEngine>(
                        new Pricers::TreeCapFloor(100));
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
                    double forward =
                        QL_LOG(p/termStructure_->discount(end))/tenor;
                    double capletValue;
                    if (start > QL_EPSILON) {
                        double d1 = (QL_LOG(forward/exerciseRate)+
                            0.5*sigma*sigma*start)/(sigma*QL_SQRT(start));
                        double d2 = d1 - sigma*QL_SQRT(start);
                        capletValue = p*tenor*
                            (forward*f(d1) - exerciseRate*f(d2));
                    } else {
                        capletValue = p*tenor*
                            QL_MAX(forward - exerciseRate, 0.0);
                    }
                    value += capletValue;
                }
                return value;
            }
        }
    }
}
