/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Ferdinando Ametrano

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


#include <ql/PricingEngines/blackmodel.hpp>
#include <ql/Solvers1D/brent.hpp>

namespace QuantLib {

    class BlackImpliedVolHelper {
      public:
        BlackImpliedVolHelper(Option::Type optionType,
                              Real strike,
                              Real forward,
                              Real blackPrice)
        : optionType_(optionType), strike_(strike), forward_(forward),
          blackPrice_(blackPrice) {}
        Real operator()(Real stdDev) const {
            return blackFormula(optionType_, strike_, forward_, stdDev) -
                blackPrice_;
        }
        Real derivative(Real stdDev) const {
            return 0.0;
        }
      private:
        Option::Type optionType_;
        Real strike_, forward_, blackPrice_;
    };

    Real blackImpliedStdDev(Option::Type optionType,
                            Real strike,
                            Real forward,
                            Real blackPrice,
                            Real guess) {
        QL_REQUIRE(blackPrice>0.0,
                   "price (" << blackPrice << ") must be positive");
        QL_REQUIRE(strike>0.0,
                   "strike (" << strike << ") must be positive");
        BlackImpliedVolHelper f(optionType, strike, forward, blackPrice);
        Brent solver;
        solver.setMaxEvaluations(100);
        Real minSdtDev = 0.0, maxStdDev = 10.0;
        if (guess==Null<Real>()) guess = 0.15;
        return solver.solve(f, 1e-10, guess, minSdtDev, maxStdDev);
    }
}
