/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
Copyright (C) 2007 Ferdinando Ametrano
Copyright (C) 2007 Mark Joshi

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

#include <ql/models/marketmodels/models/piecewiseconstantabcdvariance.hpp>
#include <ql/models/marketmodels/utilities.hpp>
#include <ql/termstructures/volatility/abcd.hpp>

namespace QuantLib {

    PiecewiseConstantAbcdVariance::PiecewiseConstantAbcdVariance(
        Real a, Real b, Real c, Real d,
        Size resetIndex,
        const std::vector<Time>& rateTimes)
        : variances_(rateTimes.size()-1, 0.0),
        volatilities_(rateTimes.size()-1, 0.0),
        rateTimes_(rateTimes) ,
        a_(a),
        b_(b),
        c_(c),
        d_(d)
    {

        checkIncreasingTimes(rateTimes);
        QL_REQUIRE(rateTimes.size()>1,
            "Rate times must contain at least two values");
        QL_REQUIRE(resetIndex<rateTimes_.size()-1,
            "resetIndex (" << resetIndex <<
            ") must be less than rateTimes.size()-1 (" <<
            rateTimes_.size()-1 << ")");
        AbcdFunction abcd(a,b,c,d);
        for (Size i=0; i<=resetIndex; ++i) {
            Time startTime = (i==0 ? 0.0 : rateTimes_[i-1]);
            variances_[i] = abcd.variance(startTime,
                rateTimes_[i],
                rateTimes_[resetIndex]);
            Time totTime = rateTimes_[i]-startTime;
            volatilities_[i] = std::sqrt(variances_[i]/totTime);
        }
    }


    void PiecewiseConstantAbcdVariance::getABCD(Real& a, Real& b, Real& c, Real& d) const
    {
        a=a_;
        b=b_;
        c=c_;
        d=d_;
    }

    const std::vector<Real>& PiecewiseConstantAbcdVariance::rateTimes() const {
        return rateTimes_;
    }

    const std::vector<Real>& PiecewiseConstantAbcdVariance::variances() const {
        return variances_;
    }

    const std::vector<Real>&
        PiecewiseConstantAbcdVariance::volatilities() const {
            return volatilities_;
    }

}
