
/*
 Copyright (C) 2003 Ferdinando Ametrano
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file europeanpathpricer.cpp
    \brief path pricer for European options
*/

#include <ql/MonteCarlo/europeanpathpricer.hpp>

namespace QuantLib {

    EuropeanPathPricer::EuropeanPathPricer(
                            Option::Type type,
                            double underlying, double strike,
                            const RelinkableHandle<TermStructure>& riskFreeTS)
    : PathPricer<Path>(riskFreeTS), underlying_(underlying),
      payoff_(type, strike) {
        QL_REQUIRE(underlying>0.0,
                   "EuropeanPathPricer: "
                   "underlying less/equal zero not allowed");
        QL_REQUIRE(strike>=0.0,
                   "EuropeanPathPricer: "
                   "strike less than zero not allowed");
    }

    double EuropeanPathPricer::operator()(const Path& path) const {
        Size n = path.size();
        QL_REQUIRE(n>0,
                   "EuropeanPathPricer: the path cannot be empty");

        double log_drift = 0.0, log_random = 0.0;
        for (Size i = 0; i < n; i++) {
            log_drift += path.drift()[i];
            log_random += path.diffusion()[i];
        }

        return payoff_(underlying_ * QL_EXP(log_drift+log_random)) *
            riskFreeTS_->discount(path.timeGrid().back());
    }

}

