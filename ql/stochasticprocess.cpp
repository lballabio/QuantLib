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
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/stochasticprocess.hpp>

namespace QuantLib {

    // base class

    StochasticProcess::StochasticProcess() {}

    StochasticProcess::StochasticProcess(
             const boost::shared_ptr<StochasticProcess::discretization>& disc)
    : discretization_(disc) {}

    Real StochasticProcess::expectation(Time t0, Real x0, Time dt) const {
        return discretization_->expectation(*this, t0, x0, dt);
    }

    Real StochasticProcess::variance(Time t0, Real x0, Time dt) const {
        return discretization_->variance(*this, t0, x0, dt);
    }

    Real StochasticProcess::evolve(Real change, Real currentValue) const {
        return currentValue + change;
    }

    Time StochasticProcess::time(const Date& d) const {
        QL_FAIL("date/time conversion not supported");
    }

    void StochasticProcess::update() {
        notifyObservers();
    }


    // Euler discretization

    Real EulerDiscretization::expectation(const StochasticProcess& process,
                                          Time t0, Real x0, Time dt) const {
        return x0 + process.drift(t0, x0)*dt;
    }

    Real EulerDiscretization::variance(const StochasticProcess& process,
                                       Time t0, Real x0, Time dt) const {
        Real sigma = process.diffusion(t0, x0);
        return sigma*sigma*dt;
    }

}
