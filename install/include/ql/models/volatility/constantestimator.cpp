/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Joseph Wang

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

#include <ql/models/volatility/constantestimator.hpp>

namespace QuantLib {
    TimeSeries<Volatility>
    ConstantEstimator::calculate(const TimeSeries<Volatility>& volatilitySeries) {
        TimeSeries<Volatility> retval;
        const std::vector<Volatility> u = volatilitySeries.values();
        TimeSeries<Volatility>::const_iterator prev, next, cur, start;
        cur = volatilitySeries.begin();
        std::advance(cur, size_);
        // ICK.  This could probably be made a lot more efficient
        for (Size i=size_; i < volatilitySeries.size(); i++) {
            Size j;
            Real sumu2=0.0, sumu=0.0;
            for (j=i-size_; j <i; j++) {
                sumu += u[j];
                sumu2 += u[j]*u[j];
            }
            Real s = std::sqrt(sumu2/(Real)size_ - sumu*sumu / (Real) size_ /
                               (Real) (size_+1));
            retval[cur->first] = s;
            ++cur;
        }
        return retval;
    }

}

