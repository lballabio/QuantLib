/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004, 2005 StatPro Italia srl

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

#include <ql/Processes/eulerdiscretization.hpp>

namespace QuantLib {

    Disposable<Array> EulerDiscretization::expectation(
                                      const GenericStochasticProcess& process,
                                      Time t0, const Array& x0,
                                      Time dt) const {
        Array result = x0;
        result += process.drift(t0, x0)*dt;
        return result;
    }

    Real EulerDiscretization::expectation(
                                      const StochasticProcess1D& process,
                                      Time t0, Real x0, Time dt) const {
        return x0 + process.drift(t0, x0)*dt;
    }

    Disposable<Matrix> EulerDiscretization::covariance(
                                      const GenericStochasticProcess& process,
                                      Time t0, const Array& x0,
                                      Time dt) const {
        Matrix sigma = process.diffusion(t0, x0);
        Matrix result = sigma*sigma*dt;
        return result;
    }

    Real EulerDiscretization::variance(const StochasticProcess1D& process,
                                       Time t0, Real x0, Time dt) const {
        Real sigma = process.diffusion(t0, x0);
        return sigma*sigma*dt;
    }

}

