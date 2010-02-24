/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004 Neil Firth
 Copyright (C) 2006 Klaus Spanderen

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

#include <ql/pricingengines/basket/mcamericanbasketengine.hpp>
#include <ql/math/functional.hpp>
#include <ql/methods/montecarlo/lsmbasissystem.hpp>
#include <boost/bind.hpp>

using boost::bind;

namespace QuantLib {

    AmericanBasketPathPricer::AmericanBasketPathPricer(
        Size assetNumber,
        const boost::shared_ptr<Payoff>& payoff,
        Size polynomOrder,
        LsmBasisSystem::PolynomType polynomType)
    : assetNumber_ (assetNumber),
      payoff_      (payoff),
      scalingValue_(1.0),
      v_           (LsmBasisSystem::multiPathBasisSystem(assetNumber_,
                                                         polynomOrder,
                                                         polynomType)) {
        QL_REQUIRE(   polynomType == LsmBasisSystem::Monomial
                   || polynomType == LsmBasisSystem::Laguerre
                   || polynomType == LsmBasisSystem::Hermite
                   || polynomType == LsmBasisSystem::Hyperbolic
                   || polynomType == LsmBasisSystem::Chebyshev2nd,
                   "insufficient polynom type");
        const boost::shared_ptr<BasketPayoff> basketPayoff
            = boost::dynamic_pointer_cast<BasketPayoff>(payoff_);
        QL_REQUIRE(basketPayoff, "payoff not a basket payoff");

        const boost::shared_ptr<StrikedTypePayoff> strikePayoff
            = boost::dynamic_pointer_cast<StrikedTypePayoff>(basketPayoff->basePayoff());

        if (strikePayoff) {
            scalingValue_/=strikePayoff->strike();
        }

        v_.push_back(boost::bind(&AmericanBasketPathPricer::payoff,
                                 this, _1));
    }

    Array AmericanBasketPathPricer::state(const MultiPath& path,
                                          Size t) const {
        QL_REQUIRE(path.assetNumber() == assetNumber_, "invalid multipath");

        Array tmp(assetNumber_);
        for (Size i=0; i<assetNumber_; ++i) {
            tmp[i] = path[i][t]*scalingValue_;
        }

        return tmp;
    }

    Real AmericanBasketPathPricer::payoff(const Array& state) const {
        const boost::shared_ptr<BasketPayoff> basketPayoff
            = boost::dynamic_pointer_cast<BasketPayoff>(payoff_);
        QL_REQUIRE(basketPayoff, "payoff not a basket payoff");

        Real value = basketPayoff->accumulate(state);
        return (*payoff_)(value/scalingValue_);
    }

    Real AmericanBasketPathPricer::operator()(const MultiPath& path,
                                              Size t) const {
        return this->payoff(this->state(path, t));
    }

    std::vector<boost::function1<Real, Array> >
    AmericanBasketPathPricer::basisSystem() const {
        return v_;
    }

}
