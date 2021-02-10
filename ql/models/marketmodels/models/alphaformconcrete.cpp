/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
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

#include <ql/models/marketmodels/models/alphaformconcrete.hpp>
#include <cmath>
#include <utility>

namespace QuantLib {

    AlphaFormInverseLinear::AlphaFormInverseLinear(std::vector<Time> times, Real alpha)
    : times_(std::move(times)), alpha_(alpha) {}

    Real AlphaFormInverseLinear::operator()(Integer i) const {
        return 1.0/(1.0+alpha_*times_[i]);
    }

    void AlphaFormInverseLinear::setAlpha(Real alpha) {
        alpha_=alpha;
    }


    AlphaFormLinearHyperbolic::AlphaFormLinearHyperbolic(std::vector<Time> times, Real alpha)
    : times_(std::move(times)), alpha_(alpha) {}

    Real AlphaFormLinearHyperbolic::operator()(Integer i) const {
        Real at = alpha_*times_[i];
        Real res = std::atan(at)-0.5*M_PI;
        res *= at;
        res += 1.0;
        res =std::sqrt(res);
        return res;
    }

    void AlphaFormLinearHyperbolic::setAlpha(Real alpha) {
        alpha_ = alpha;
    }

}
