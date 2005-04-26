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

#include <ql/Processes/ornsteinuhlenbeckprocess.hpp>

namespace QuantLib {

    OrnsteinUhlenbeckProcess::OrnsteinUhlenbeckProcess(Real speed,
                                                       Volatility vol,
                                                       Real x0)
    : x0_(x0), speed_(speed), volatility_(vol) {}

    Real OrnsteinUhlenbeckProcess::x0() const {
        return x0_;
    }

    Real OrnsteinUhlenbeckProcess::drift(Time, Real x) const {
        return - speed_*x;
    }

    Real OrnsteinUhlenbeckProcess::diffusion(Time, Real) const {
        return volatility_;
    }

    Real OrnsteinUhlenbeckProcess::expectation(Time, Real x0,
                                               Time dt) const {
        return x0*std::exp(-speed_*dt);
    }

    Real OrnsteinUhlenbeckProcess::stdDeviation(Time t, Real x0,
                                                Time dt) const {
        return std::sqrt(variance(t,x0,dt));
    }

    Real OrnsteinUhlenbeckProcess::variance(Time, Real, Time dt) const {
        return 0.5*volatility_*volatility_/speed_*
            (1.0 - std::exp(-2.0*speed_*dt));
    }

}
