
/*
 Copyright (C) 2002, 2003 Ferdinando Ametrano

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

#include <ql/Instruments/forwardvanillaoption.hpp>

namespace QuantLib {

    ForwardVanillaOption::ForwardVanillaOption(
        Real moneyness,
        Date resetDate,
        const boost::shared_ptr<StochasticProcess>& process,
        const boost::shared_ptr<StrikedTypePayoff>& payoff,
        const boost::shared_ptr<Exercise>& exercise,
        const boost::shared_ptr<PricingEngine>& engine)
    : VanillaOption(process, payoff, exercise, engine),
      moneyness_(moneyness), resetDate_(resetDate) {}

    void ForwardVanillaOption::setupArguments(Arguments* args) const {
        VanillaOption::setupArguments(args);
        ForwardVanillaOption::arguments* arguments =
            dynamic_cast<ForwardVanillaOption::arguments*>(args);
        QL_REQUIRE(arguments != 0, "wrong argument type");

        arguments->moneyness = moneyness_;
        arguments->resetDate = resetDate_;

    }

    void ForwardVanillaOption::performCalculations() const {
        if (isExpired()) {
            NPV_ = delta_ = gamma_ = theta_ =
                vega_ =   rho_ = dividendRho_ = strikeSensitivity_ = 0.0;
        } else {
            Option::performCalculations();

            const ForwardVanillaOption::results* results =
                dynamic_cast<const ForwardVanillaOption::results*>(
                                                          engine_->results());
            QL_ENSURE(results != 0,
                      "no results returned from pricing engine");
            delta_       = results->delta;
            gamma_       = results->gamma;
            theta_       = results->theta;
            vega_        = results->vega;
            rho_         = results->rho;
            dividendRho_ = results->dividendRho;

        }
    }

}

