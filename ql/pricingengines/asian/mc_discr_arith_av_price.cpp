/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004 Ferdinando Ametrano

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/pricingengines/asian/mc_discr_geom_av_price.hpp>
#include <ql/pricingengines/asian/mc_discr_arith_av_price.hpp>

namespace QuantLib {

    ArithmeticAPOPathPricer::ArithmeticAPOPathPricer(
                                         Option::Type type,
                                         Real strike, DiscountFactor discount,
                                         Real runningSum, Size pastFixings)
    : payoff_(type, strike), discount_(discount),
      runningSum_(runningSum), pastFixings_(pastFixings) {
        QL_REQUIRE(strike>=0.0,
            "strike less than zero not allowed");
    }

    Real ArithmeticAPOPathPricer::operator()(const Path& path) const  {
        Size n = path.length();
        QL_REQUIRE(n>1, "the path cannot be empty");

        Real sum;
        Size fixings;
        if (path.timeGrid().mandatoryTimes()[0]==0.0) {
            // include initial fixing
            sum = std::accumulate(path.begin(),path.end(),runningSum_);
            fixings = pastFixings_ + n;
        } else {
            sum = std::accumulate(path.begin()+1,path.end(),runningSum_);
            fixings = pastFixings_ + n - 1;
        }
        Real averagePrice = sum/fixings;
        return discount_ * payoff_(averagePrice);
    }

}
