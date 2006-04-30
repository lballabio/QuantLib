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

/*! \file simplelocalestimator.hpp
    \brief Constant volatility estimator
*/

#ifndef quantlib_simple_local_estimator_hpp
#define quantlib_simple_local_estimator_hpp

#include <ql/volatilitymodel.hpp>
#include <map>

namespace QuantLib {

    /*! This class implements a concrete volatility model

        Volatilities are assumed to be expressed on an annual basis.
    */
    class SimpleLocalEstimator : 
        public LocalVolatilityEstimator<Real> {
    private:
        Real yearFraction_;
      public:
        SimpleLocalEstimator(Real y) :
        yearFraction_(y) {}
        TimeSeries<Volatility>
        calculate(const TimeSeries<Real> &quoteSeries) {
            TimeSeries<Volatility> retval;
            TimeSeries<Volatility>::const_valid_iterator 
                prev, next, cur, start;
            start = quoteSeries.vbegin();
            start++;
            for (cur = start;
                 cur != quoteSeries.vend();
                 cur++) {
                prev = cur; prev--;
                retval.insert(cur->first, 
                              std::abs(std::log(cur->second/
                                     prev->second)));
            }
            return retval;
        }
    };
}


#endif
