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
/*! \file blackcapfloor.cpp
    \brief European capfloor calculated using Black formula

    \fullpath
    ql/Pricers/%blackcapfloor.cpp
*/

// $Id$

#include "ql/Pricers/blackcapfloor.hpp"

namespace QuantLib {

    namespace Pricers {

        using Instruments::VanillaCapFloor;
        using InterestRateModelling::BlackModel;

        void BlackCapFloor::calculate() const {
            double value = 0.0;
            Instruments::VanillaCapFloor::Type type = parameters_.type;

            for (Size i=0; i<parameters_.startTimes.size(); i++) {
                Time start = parameters_.startTimes[i];
                Time end = parameters_.endTimes[i];
                double tenor = end - start;
                double p = model_->termStructure()->discount(start);
                double q = model_->termStructure()->discount(end);
                double forward = QL_LOG(p/q)/tenor;
                if ((type == Instruments::VanillaCapFloor::Cap) ||
                    (type == Instruments::VanillaCapFloor::Collar)) {
                    value +=  q*tenor*BlackModel::formula(
                        parameters_.capRates[i], forward, 
                        model_->volatility()*QL_SQRT(start), 1);

                }
                if ((type == Instruments::VanillaCapFloor::Floor) ||
                    (type == Instruments::VanillaCapFloor::Collar)) {
                    value +=  q*tenor*BlackModel::formula(
                        parameters_.floorRates[i], forward, 
                        model_->volatility()*QL_SQRT(start), -1);
                }
            }
            results_.value = value;

        }

    }

}
