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
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/


#ifndef quantlib_piecewise_const_variance_hpp
#define quantlib_piecewise_const_variance_hpp

#include <ql/types.hpp>
#include <vector>

namespace QuantLib {

    class EvolutionDescription;
    class Matrix;

    class PiecewiseConstantVariance {
      public:
        virtual ~PiecewiseConstantVariance() = default;
        virtual const std::vector<Real>& variances() const = 0;
        virtual const std::vector<Volatility>& volatilities() const = 0;
        virtual const std::vector<Time>& rateTimes() const = 0;
        Real variance(Size i) const;
        Volatility volatility(Size i) const;
        //const std::vector<Real>& totalVariances() const;
        //const std::vector<Volatility>& totalVolatilities() const;
        Real totalVariance(Size i) const;
        Volatility totalVolatility(Size i) const;
    };

}

#endif


#ifndef id_c2b879349e3cf2ba5ed259cc586d2287
#define id_c2b879349e3cf2ba5ed259cc586d2287
inline bool test_c2b879349e3cf2ba5ed259cc586d2287(const int* i) {
    return i != nullptr;
}
#endif
