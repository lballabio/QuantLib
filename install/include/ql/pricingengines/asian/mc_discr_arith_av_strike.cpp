/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Master IMAFA - Polytech'Nice Sophia - Université de Nice Sophia Antipolis

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

#include <ql/pricingengines/asian/mc_discr_arith_av_strike.hpp>

namespace QuantLib {

    ArithmeticASOPathPricer::ArithmeticASOPathPricer(Option::Type type,
                                                     DiscountFactor discount,
                                                     Real runningSum,
                                                     Size pastFixings)
    : type_(type), discount_(discount),
      runningSum_(runningSum), pastFixings_(pastFixings) {}


    Real ArithmeticASOPathPricer::operator()(const Path& path) const  {
        Size n = path.length();
        QL_REQUIRE(n > 1, "the path cannot be empty");

        Real averageStrike;
        if (path.timeGrid().mandatoryTimes()[0]==0.0) {
            // include initial fixing
            averageStrike =
                std::accumulate(path.begin(),path.end(),runningSum_) /
                (pastFixings_ + n);
        } else {
            averageStrike =
                std::accumulate(path.begin()+1,path.end(),runningSum_) /
                (pastFixings_ + n - 1);
        }

        return discount_
            * PlainVanillaPayoff(type_, averageStrike)(path.back());
    }

}

