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

#include <ql/processes/squarerootprocess.hpp>

namespace QuantLib {

    SquareRootProcess::SquareRootProcess(
             Real b, Real a, Volatility sigma, Real x0,
             const std::shared_ptr<discretization>& disc)
    : StochasticProcess1D(disc), x0_(x0), mean_(b), speed_(a),
      volatility_(sigma) {}

    Real SquareRootProcess::x0() const {
        return x0_;
    }

    Real SquareRootProcess::drift(Time, Real x) const {
        return speed_*(mean_ - x);
    }

    Real SquareRootProcess::diffusion(Time, Real x) const {
        return volatility_*std::sqrt(x);
    }

}
