/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Klaus Spanderen

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

/*! \file mcamericanengine.cpp
    \brief Monte Carlo engine for vanilla american options
*/

#include <boost/bind.hpp>

#include <ql/errors.hpp>
#include <ql/Math/functional.hpp>
#include <ql/Instruments/payoffs.hpp>
#include <ql/PricingEngines/Vanilla/mcamericanengine.hpp>

using boost::bind;

namespace QuantLib {

    AmericanPathPricer::AmericanPathPricer(
        const boost::shared_ptr<Payoff>& payoff,
        Size polynomOrder,
        LsmBasisSystem::PolynomType polynomType) 
    : scalingValue_(1.0),
      payoff_      (payoff),
      v_           (LsmBasisSystem::pathBasisSystem(polynomOrder, 
                                                    polynomType)) {
        
        QL_REQUIRE(   polynomType == LsmBasisSystem::Monomial
                   || polynomType == LsmBasisSystem::Laguerre
                   || polynomType == LsmBasisSystem::Hermite
                   || polynomType == LsmBasisSystem::Hyperbolic
                   || polynomType == LsmBasisSystem::Chebyshev2th,
                   "insufficient polynom type");
                    
        // the payoff gives an additional value
        v_.push_back(boost::bind(&AmericanPathPricer::payoff, this, _1));

        const boost::shared_ptr<StrikedTypePayoff> strikePayoff 
            = boost::dynamic_pointer_cast<StrikedTypePayoff>(payoff_);

        if (strikePayoff) {
            scalingValue_/=strikePayoff->strike();
        }      
    }

    Real AmericanPathPricer::payoff(Real state) const {
        return (*payoff_)(state/scalingValue_);
    }

    Real AmericanPathPricer::operator()(const Path& path, Size t) const {
        return this->payoff(this->state(path, t));
    }

    Real AmericanPathPricer::state(const Path& path, Size t) const {
        // scale values of the underlying 
        // to increase numerical stability
        return path[t]*scalingValue_;
    }

    std::vector<boost::function1<Real, Real> > 
    AmericanPathPricer::basisSystem() const {
        return v_;
    }
}

