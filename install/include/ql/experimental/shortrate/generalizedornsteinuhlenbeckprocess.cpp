/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2010 SunTrust Bank
 Copyright (C) 2010 Cavit Hafizoglu

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

#include <ql/experimental/shortrate/generalizedornsteinuhlenbeckprocess.hpp>
#include <utility>

namespace QuantLib {

    GeneralizedOrnsteinUhlenbeckProcess::GeneralizedOrnsteinUhlenbeckProcess(
        std::function<Real(Time)> speed, std::function<Real(Time)> vol, Real x0, Real level)
    : x0_(x0), level_(level), speed_(std::move(speed)), volatility_(std::move(vol)) {

        QL_REQUIRE(x0 >= 0.0, "negative initial data given");
        QL_REQUIRE(level >= 0.0, "negative level given");
    }

    Real GeneralizedOrnsteinUhlenbeckProcess::x0() const {
        return x0_;
    }

    Real GeneralizedOrnsteinUhlenbeckProcess::drift(Time t, Real x) const {
        return speed_(t) * (level_ - x);;
    }

    Real GeneralizedOrnsteinUhlenbeckProcess::diffusion(Time t, Real) const {
        return volatility_(t);
    }

    Real GeneralizedOrnsteinUhlenbeckProcess::expectation(
                                             Time t, Real x0, Time dt) const {
        return level_ + (x0 - level_) * std::exp(-speed_(t)*dt);
    }

    Real GeneralizedOrnsteinUhlenbeckProcess::stdDeviation(
                                             Time t, Real x0, Time dt) const {
        return std::sqrt(variance(t,x0,dt));
    }

    Real GeneralizedOrnsteinUhlenbeckProcess::variance(
                                              Time t, Real, Time dt) const {
        Real speed = speed_(t);
        Volatility vol = volatility_(t);

        if (speed < std::sqrt(QL_EPSILON)) {
            // algebraic limit for small speed
            return vol*vol*dt;
        } else {
            return 0.5*vol*vol/speed*(1.0 - std::exp(-2.0*speed*dt));
        }
    }


    Real GeneralizedOrnsteinUhlenbeckProcess::speed(Time t) const {
        return speed_(t);
    }

    Real GeneralizedOrnsteinUhlenbeckProcess::volatility(Time t) const {
        return volatility_(t);
    }

    Real GeneralizedOrnsteinUhlenbeckProcess::level() const {
        return level_;
    }

}

