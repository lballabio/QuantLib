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
/*! \file capfloor.cpp
    \brief European cap and floor class

    \fullpath
    ql/Instruments/%capfloor.cpp
*/

// $Id$

#include <ql/Pricers/capfloorpricer.hpp>

namespace QuantLib {

    namespace Pricers {

        void DiscretizedCapFloor::applyCondition() {
            for (Size i=0; i<parameters_.startTimes.size(); i++) {
                if (isOnTime(parameters_.startTimes[i])) {
                    Time end = parameters_.endTimes[i];
                    Handle<DiscretizedAsset> bond(new 
                        DiscretizedDiscountBond(method()));
                    method()->initialize(bond, end);
                    method()->rollback(bond,time_);

                    Instruments::VanillaCapFloor::Type type = 
                        parameters_.type;

                    if ( (type == Instruments::VanillaCapFloor::Cap) ||
                         (type == Instruments::VanillaCapFloor::Collar)) {
                        double accrual = 1.0 + 
                            parameters_.capRates[i]*(end - time_);
                        double strike = 1.0/accrual;
                        for (Size j=0; j<values_.size(); j++)
                            values_[j] += parameters_.nominals[i]*accrual*
                                QL_MAX(strike - bond->values()[j], 0.0);
                    }

                    if ( (type == Instruments::VanillaCapFloor::Floor) ||
                         (type == Instruments::VanillaCapFloor::Collar)) {
                        double accrual = 1.0 + 
                            parameters_.floorRates[i]*(end - time_);
                        double strike = 1.0/accrual;
                        for (Size j=0; j<values_.size(); j++)
                            values_[j] += parameters_.nominals[i]*accrual*
                                QL_MAX(bond->values()[j] - strike, 0.0);
                    }

                }
            }
        }
    
    }

}
