/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007, 2008 Klaus Spanderen

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

/*! \file mchestonhullwhiteengine.hpp
    \brief Monte Carlo vanilla option engine for stochastic interest rates
*/

#include <ql/pricingengines/vanilla/mchestonhullwhiteengine.hpp>
#include <utility>

namespace QuantLib {

    HestonHullWhitePathPricer::HestonHullWhitePathPricer(
        Time exerciseTime,
        ext::shared_ptr<Payoff> payoff,
        ext::shared_ptr<HybridHestonHullWhiteProcess> process)
    : exerciseTime_(exerciseTime), payoff_(std::move(payoff)), process_(std::move(process)) {}

    Real HestonHullWhitePathPricer::operator()(const MultiPath& path) const {
        QL_REQUIRE(path.pathSize() > 0, "the path cannot be empty");

        Array states(path.assetNumber());
        for (Size j=0; j < states.size(); ++j) {
            states[j] = path[j][path.pathSize()-1];
        }

        const DiscountFactor df(
                             1.0/process_->numeraire(exerciseTime_, states));
        return (*payoff_)(states[0])*df;
    }
}

