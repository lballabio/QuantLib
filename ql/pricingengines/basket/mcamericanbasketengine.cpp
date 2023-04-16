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

#include <ql/methods/montecarlo/lsmbasissystem.hpp>
#include <ql/pricingengines/basket/mcamericanbasketengine.hpp>
#include <utility>

namespace QuantLib {

    AmericanBasketPathPricer::AmericanBasketPathPricer(
        Size assetNumber,
        std::shared_ptr<Payoff> payoff,
        Size polynomialOrder,
        LsmBasisSystem::PolynomialType polynomialType)
    : assetNumber_(assetNumber), payoff_(std::move(payoff)),
      v_(LsmBasisSystem::multiPathBasisSystem(assetNumber_, polynomialOrder, polynomialType)) {
        QL_REQUIRE(   polynomialType == LsmBasisSystem::Monomial
                   || polynomialType == LsmBasisSystem::Laguerre
                   || polynomialType == LsmBasisSystem::Hermite
                   || polynomialType == LsmBasisSystem::Hyperbolic
                   || polynomialType == LsmBasisSystem::Chebyshev2nd,
                   "insufficient polynomial type");

        const std::shared_ptr<BasketPayoff> basketPayoff
            = std::dynamic_pointer_cast<BasketPayoff>(payoff_);
        QL_REQUIRE(basketPayoff, "payoff not a basket payoff");

        const std::shared_ptr<StrikedTypePayoff> strikePayoff
            = std::dynamic_pointer_cast<StrikedTypePayoff>(basketPayoff->basePayoff());

        if (strikePayoff != nullptr) {
            scalingValue_/=strikePayoff->strike();
        }

        v_.emplace_back([&](const Array& state) { return this->payoff(state); });
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
        const std::shared_ptr<BasketPayoff> basketPayoff
            = std::dynamic_pointer_cast<BasketPayoff>(payoff_);
        QL_REQUIRE(basketPayoff, "payoff not a basket payoff");

        Real value = basketPayoff->accumulate(state);
        return (*payoff_)(value/scalingValue_);
    }

    Real AmericanBasketPathPricer::operator()(const MultiPath& path,
                                              Size t) const {
        return this->payoff(this->state(path, t));
    }

    std::vector<std::function<Real(Array)> >
    AmericanBasketPathPricer::basisSystem() const {
        return v_;
    }

}
