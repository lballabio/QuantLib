/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb
 Copyright (C) 2003 Ferdinando Ametrano
 Copyright (C) 2004, 2005, 2006, 2007 StatPro Italia srl

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

#include <ql/processes/ornsteinuhlenbeckprocess.hpp>

namespace QuantLib {

    OrnsteinUhlenbeckProcess::OrnsteinUhlenbeckProcess(Real speed,
                                                       Volatility vol,
                                                       Real x0,
                                                       Real level)
    : x0_(x0), speed_(speed), level_(level), volatility_(vol) {
        QL_REQUIRE(speed_ >= 0.0, "negative speed given");
        QL_REQUIRE(volatility_ >= 0.0, "negative volatility given");
    }

    Real OrnsteinUhlenbeckProcess::variance(Time, Real, Time dt) const {
        if (speed_ < std::sqrt(QL_EPSILON)) {
             // algebraic limit for small speed
            return volatility_*volatility_*dt;
        } else {
            return 0.5*volatility_*volatility_/speed_*
                (1.0 - std::exp(-2.0*speed_*dt));
        }
    }

}

