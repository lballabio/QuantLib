
/*
 Copyright (C) 2004 StatPro Italia srl

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

#include <ql/Instruments/europeanoption.hpp>
#include <ql/PricingEngines/Vanilla/analyticeuropeanengine.hpp>

namespace QuantLib {

    EuropeanOption::EuropeanOption(
        const boost::shared_ptr<BlackScholesStochasticProcess>& stochProc,
        const boost::shared_ptr<StrikedTypePayoff>& payoff,
        const boost::shared_ptr<Exercise>& exercise,
        const boost::shared_ptr<PricingEngine>& engine)
    : VanillaOption(stochProc, payoff, exercise, engine) {
        if (!engine)
            setPricingEngine(boost::shared_ptr<PricingEngine>(
                                                 new AnalyticEuropeanEngine));
    }

}

