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

#include <ql/processes/geometricbrownianprocess.hpp>
#include <ql/processes/eulerdiscretization.hpp>

namespace QuantLib {

    GeometricBrownianMotionProcess::GeometricBrownianMotionProcess(
                                                          Real initialValue,
                                                          Real mue,
                                                          Real sigma)
    : StochasticProcess1D(ext::shared_ptr<discretization>(
                                                    new EulerDiscretization)),
      initialValue_(initialValue), mue_(mue), sigma_(sigma) {}

    Real GeometricBrownianMotionProcess::x0() const {
        return initialValue_;
    }

    Real GeometricBrownianMotionProcess::drift(Time, Real x) const {
        return mue_ * x;
    }

    Real GeometricBrownianMotionProcess::diffusion(Time, Real x) const {
        return sigma_ * x;
    }

}
