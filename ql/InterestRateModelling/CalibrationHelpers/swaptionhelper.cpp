
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
/*! \file swaptionhelper.cpp
    \brief Swaption calibration helper

    \fullpath
    ql/InterestRateModelling/CalibrationHelpers/%swaptionhelper.cpp
*/

// $Id$

#include "ql/CashFlows/floatingratecoupon.hpp"
#include "ql/Math/normaldistribution.hpp"
#include "ql/InterestRateModelling/CalibrationHelpers/swaptionhelper.hpp"
#include "ql/Pricers/jamshidianswaption.hpp"
#include "ql/Pricers/treeswaption.hpp"

#include <iostream>

namespace QuantLib {

    namespace InterestRateModelling {

        namespace CalibrationHelpers {

            using Instruments::SimpleSwap;
            using Instruments::Swaption;
            using Instruments::SwaptionParameters;

            using Pricers::SwaptionPricingEngine;
            using Pricers::JamshidianSwaption;
            using Pricers::TreeSwaption;

            SwaptionHelper::SwaptionHelper(
                const Period& maturity,
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
                Date startDate = termStructure->settlementDate().
                    plus(maturity.length(), maturity.units());
                Rate fixedRate = 0.04;//dummy value
                swap_ = Handle<SimpleSwap>(new SimpleSwap(
                  false,
                  startDate,
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
                  termStructure));
                Rate fairFixedRate = fixedRate -
                    swap_->NPV()/swap_->fixedLegBPS();
                swap_ = Handle<SimpleSwap>(new SimpleSwap(
                  false,
                  startDate,
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
                exerciseRate_ = fairFixedRate;

                engine_ = Handle<SwaptionPricingEngine>(new
                    TreeSwaption(100));

                swaption_ = Handle<Swaption>(new
                    Swaption(*swap_, EuropeanExercise(startDate), termStructure,
                             engine_));
                swaption_->setPricingEngine(engine_);
            }

            double SwaptionHelper::modelValue(const Handle<Model>& model) {
/*                if (model->hasDiscountBondFormula() &&
                    model->hasDiscountBondOptionFormula())
                    engine_ = Handle<SwaptionPricingEngine>(new
                        JamshidianSwaption());
                else*/
                engine_ = Handle<SwaptionPricingEngine>(new
                    TreeSwaption(100));
                engine_->setModel(model);
                swaption_->setPricingEngine(engine_);
                swaption_->recalculate();
                return swaption_->NPV();
            }

            double SwaptionHelper::blackPrice(double sigma) const {
                //FIXME: not completed, to check
                SwaptionParameters* params = dynamic_cast<SwaptionParameters*>(
                    engine_->parameters());
                QL_REQUIRE(params!=0, "These are not swaption parameters");
                const std::vector<Time>& times = params->fixedPayTimes;
                double p = 0.0;
                for (size_t i=0; i<times.size(); i++) {
                    p += termStructure_->discount(times[i]);
                }
                double swapRate =
                    exerciseRate_ - swap_->NPV()/swap_->fixedLegBPS();
                Time start = params->floatingResetTimes[0];
                double value;
                if (start>0.0) {
                    Math::CumulativeNormalDistribution f;
                    double d1 = QL_LOG(swapRate/exerciseRate_)/
                        (sigma*QL_SQRT(start))
                        + 0.5*sigma*QL_SQRT(start);
                    double d2 = d1 - sigma*QL_SQRT(start);
                    value = p*(swapRate*f(d1) - exerciseRate_*f(d2));
                } else {
                    value = p*QL_MAX(swapRate - exerciseRate_, 0.0);
                }
                return value;
            }
        }
    }
}
