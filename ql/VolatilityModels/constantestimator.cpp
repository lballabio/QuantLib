/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Joseph Wang

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/VolatilityModels/constantestimator.hpp>

namespace QuantLib {

    TimeSeries<Volatility>
    ConstantEstimator::calculate(const TimeSeries<Real>& quoteSeries) {
        TimeSeries<Volatility> retval;
        std::vector<Real> u;
        Size i;
        TimeSeries<Real>::const_valid_iterator prev, next, cur;
        for (cur = quoteSeries.vbegin();
             cur != quoteSeries.vend();
             cur++) {
            prev = cur; prev--;
            u.push_back(std::log(cur->second/
                                 prev->second));
        }
        cur = quoteSeries.vbegin();
        std::advance(cur, size_);
        for (i=size_; i < quoteSeries.size(); i++) {
            Size j;
            Real sumu2=0.0, sumu=0.0;
            for (j=i-size_; j <i; j++) {
                sumu += u[i];
                sumu2 += u[i]*u[i];
            }
            Real s = std::sqrt(sumu2/(Real)size_ - sumu / (Real) size_ /
                               (Real) (size_+1));
            retval.insert(cur->first,
                          s / std::sqrt(yearFraction_));
            cur++;
        }
        return retval;
    }

}

