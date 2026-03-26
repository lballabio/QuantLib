/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Master IMAFA - Polytech'Nice Sophia - Université de Nice Sophia Antipolis

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

#include <ql/pricingengines/asian/mc_discr_arith_av_strike.hpp>

namespace QuantLib {

    ArithmeticASOPathPricer::ArithmeticASOPathPricer(Option::Type type,
                                                     DiscountFactor discount,
                                                     Real runningSum,
                                                     Size pastFixings,
                                                     Size fixingCount)
    : type_(type), discount_(discount),
      runningSum_(runningSum), pastFixings_(pastFixings),
      fixingCount_(fixingCount) {}


    Real ArithmeticASOPathPricer::operator()(const Path& path) const  {
        Size n = path.length();
        QL_REQUIRE(n > 1, "the path cannot be empty");

        // When fixingCount_ is set, the path may include an extra point
        // at the exercise date beyond the last fixing date.  Average
        // only over the fixing points; use path.back() for the spot
        // at exercise.
        Size nFixings = (fixingCount_ != Null<Size>()) ? fixingCount_ : n;
        QL_REQUIRE(nFixings <= n, "fixingCount (" << nFixings
                   << ") exceeds path length (" << n << ")");

        Real averageStrike;
        if (path.timeGrid().mandatoryTimes()[0]==0.0) {
            // include initial fixing (T=0 is a fixing date)
            averageStrike =
                std::accumulate(path.begin(),
                                path.begin() + nFixings,
                                runningSum_) /
                (pastFixings_ + nFixings);
        } else {
            // first path point is T=0 (not a fixing), skip it
            averageStrike =
                std::accumulate(path.begin()+1,
                                path.begin() + nFixings,
                                runningSum_) /
                (pastFixings_ + nFixings - 1);
        }

        return discount_
            * PlainVanillaPayoff(type_, averageStrike)(path.back());
    }

}

