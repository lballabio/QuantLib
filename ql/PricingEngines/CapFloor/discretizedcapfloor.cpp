
/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/PricingEngines/CapFloor/discretizedcapfloor.hpp>

namespace QuantLib {

    void DiscretizedCapFloor::preAdjustValues() {
        for (Size i=0; i<arguments_.startTimes.size(); i++) {
            if (isOnTime(arguments_.startTimes[i])) {
                Time end = arguments_.endTimes[i];
                Time tenor = arguments_.accrualTimes[i];
                boost::shared_ptr<DiscretizedAsset> bond(
                                       new DiscretizedDiscountBond(method()));
                method()->initialize(bond, end);
                method()->rollback(bond,time_);

                CapFloor::Type type = arguments_.type;

                if ( (type == CapFloor::Cap) ||
                     (type == CapFloor::Collar)) {
                    Real accrual = 1.0 + arguments_.capRates[i]*tenor;
                    Real strike = 1.0/accrual;
                    for (Size j=0; j<values_.size(); j++)
                        values_[j] += arguments_.nominals[i]*accrual*
                            QL_MAX<Real>(strike - bond->values()[j], 0.0);
                }

                if ( (type == CapFloor::Floor) ||
                     (type == CapFloor::Collar)) {
                    Real accrual = 1.0 + arguments_.floorRates[i]*tenor;
                    Real strike = 1.0/accrual;
                    Real mult = (type == CapFloor::Floor)?1.0:-1.0;
                    for (Size j=0; j<values_.size(); j++)
                        values_[j] += arguments_.nominals[i]*accrual*mult*
                            QL_MAX<Real>(bond->values()[j] - strike, 0.0);
                }

            }
        }
    }

}
