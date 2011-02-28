/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2002, 2003 Ferdinando Ametrano
 Copyright (C) 2007 StatPro Italia srl

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

#include <ql/instruments/forwardvanillaoption.hpp>

namespace QuantLib {

    ForwardVanillaOption::ForwardVanillaOption(
                           Real moneyness,
                           const Date& resetDate,
                           const boost::shared_ptr<StrikedTypePayoff>& payoff,
                           const boost::shared_ptr<Exercise>& exercise)
    : OneAssetOption(payoff, exercise),
      moneyness_(moneyness), resetDate_(resetDate) {}

    void ForwardVanillaOption::setupArguments(
                                       PricingEngine::arguments* args) const {
        OneAssetOption::setupArguments(args);
        ForwardVanillaOption::arguments* arguments =
            dynamic_cast<ForwardVanillaOption::arguments*>(args);
        QL_REQUIRE(arguments != 0, "wrong argument type");

        arguments->moneyness = moneyness_;
        arguments->resetDate = resetDate_;

    }

    void ForwardVanillaOption::fetchResults(
                                      const PricingEngine::results* r) const {
        OneAssetOption::fetchResults(r);
        const ForwardVanillaOption::results* results =
            dynamic_cast<const ForwardVanillaOption::results*>(r);
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

