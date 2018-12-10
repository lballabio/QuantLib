/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005 StatPro Italia srl

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

#include <ql/pricingengines/greeks.hpp>
#include <ql/processes/blackscholesprocess.hpp>

namespace QuantLib {

    Real blackScholesTheta(
                   const ext::shared_ptr<GeneralizedBlackScholesProcess>& p,
                   Real value, Real delta, Real gamma) {

        Real u = p->stateVariable()->value();
        Rate r = p->riskFreeRate()->zeroRate(0.0, Continuous);
        Rate q = p->dividendYield()->zeroRate(0.0, Continuous);
        Volatility v = p->localVolatility()->localVol(0.0, u);

        return r*value -(r-q)*u*delta - 0.5*v*v*u*u*gamma;
    }

    Real defaultThetaPerDay(Real theta) {
        return theta/365.0;
    }

}
