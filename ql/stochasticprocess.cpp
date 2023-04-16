/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 Ferdinando Ametrano
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb
 Copyright (C) 2004, 2005 StatPro Italia srl

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

#include <ql/stochasticprocess.hpp>
#include <utility>

namespace QuantLib {

    // base class

    StochasticProcess::StochasticProcess(std::shared_ptr<discretization> disc)
    : discretization_(std::move(disc)) {}

    Size StochasticProcess::factors() const {
        return size();
    }

    Array StochasticProcess::expectation(Time t0,
                                         const Array& x0,
                                         Time dt) const {
        return apply(x0, discretization_->drift(*this, t0, x0, dt));
    }

    Matrix StochasticProcess::stdDeviation(Time t0,
                                           const Array& x0,
                                           Time dt) const {
        return discretization_->diffusion(*this, t0, x0, dt);
    }

    Matrix StochasticProcess::covariance(Time t0,
                                         const Array& x0,
                                         Time dt) const {
        return discretization_->covariance(*this, t0, x0, dt);
    }

    Array StochasticProcess::evolve(Time t0, const Array& x0,
                                    Time dt, const Array& dw) const {
        return apply(expectation(t0,x0,dt), stdDeviation(t0,x0,dt)*dw);
    }

    Array StochasticProcess::apply(const Array& x0,
                                   const Array& dx) const {
        return x0 + dx;
    }

    Time StochasticProcess::time(const Date& ) const {
        QL_FAIL("date/time conversion not supported");
    }

    void StochasticProcess::update() {
        notifyObservers();
    }


    // 1-D specialization

    StochasticProcess1D::StochasticProcess1D(std::shared_ptr<discretization> disc)
    : discretization_(std::move(disc)) {}

    Real StochasticProcess1D::expectation(Time t0, Real x0, Time dt) const {
        return apply(x0, discretization_->drift(*this, t0, x0, dt));
    }

    Real StochasticProcess1D::stdDeviation(Time t0, Real x0, Time dt) const {
        return discretization_->diffusion(*this, t0, x0, dt);
    }

    Real StochasticProcess1D::variance(Time t0, Real x0, Time dt) const {
        return discretization_->variance(*this, t0, x0, dt);
    }

    Real StochasticProcess1D::evolve(Time t0, Real x0,
                                     Time dt, Real dw) const {
        return apply(expectation(t0,x0,dt), stdDeviation(t0,x0,dt)*dw);
    }

    Real StochasticProcess1D::apply(Real x0, Real dx) const {
        return x0 + dx;
    }

}
