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
/*! \file blackswaption.cpp
    \brief European swaption calculated using Black formula

    \fullpath
    ql/Pricers/%blackswaption.cpp
*/

// $Id$

#include "ql/Pricers/blackswaption.hpp"

namespace QuantLib {

    namespace Pricers {

        using InterestRateModelling::BlackModel;

        void BlackSwaption::calculate() const {
            QL_REQUIRE(parameters_.isVanilla, "Not a vanilla swaption!");
            const std::vector<Time>& times = parameters_.floatingPayTimes;
            double p = 0.0;
            for (Size i=0; i<times.size(); i++) {
                Time tenor = times[i] - parameters_.floatingResetTimes[i];
                p += tenor*model_->termStructure()->discount(times[i]);
            }
            Time start = parameters_.floatingResetTimes[0];
            double w;
            if (parameters_.payFixed)
                w = 1.0;
            else 
                w = -1.0;
            results_.value =  p*parameters_.nominals.front()*
                BlackModel::formula(parameters_.fixedRate, parameters_.fairRate,
                                    model_->volatility()*QL_SQRT(start), w);
        }

    }

}
