/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Mark Joshi

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/models/marketmodels/discounter.hpp>
#include <ql/models/marketmodels/curvestate.hpp>
#include <ql/models/marketmodels/utilities.hpp>
#include <algorithm>

namespace QuantLib {

    MarketModelDiscounter::MarketModelDiscounter(
                                        Time paymentTime,
                                        const std::vector<Time>& rateTimes) {
        checkIncreasingTimes(rateTimes);
        before_ = std::lower_bound(rateTimes.begin(), rateTimes.end(),
                                   paymentTime) - rateTimes.begin();

        // handle the case where the payment is in the last
        // period or after the last period
        if (before_ > rateTimes.size()-2)
            before_ =  rateTimes.size()-2;

        beforeWeight_=1.0-(paymentTime-rateTimes[before_])/
            (rateTimes[before_+1]-rateTimes[before_]);
    }

    Real MarketModelDiscounter::numeraireBonds(const CurveState& curveState,
                                               Size numeraire) const {
        Real preDF = curveState.discountRatio(before_,numeraire);
        if (beforeWeight_==1.0)
            return preDF;

        Real postDF = curveState.discountRatio(before_+1,numeraire);
        if (beforeWeight_==0.0)
            return postDF;

        return std::pow(preDF,beforeWeight_)*std::pow(postDF,1.-beforeWeight_);
    }

}
