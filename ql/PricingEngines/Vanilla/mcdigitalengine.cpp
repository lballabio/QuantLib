
/*
 Copyright (C) 2004 Ferdinando Ametrano
 Copyright (C) 2003 Neil Firth
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

#include <ql/PricingEngines/Vanilla/mcdigitalengine.hpp>

namespace QuantLib {

    DigitalPathPricer::DigitalPathPricer(
                      const boost::shared_ptr<CashOrNothingPayoff>& payoff,
                      const boost::shared_ptr<AmericanExercise>& exercise,
                      Real underlying,
                      const Handle<YieldTermStructure>& discountTS,
                      const boost::shared_ptr<StochasticProcess>& diffProcess,
                      const PseudoRandom::ursg_type& sequenceGen)
    : payoff_(payoff), exercise_(exercise),
      underlying_(underlying), diffProcess_(diffProcess),
      sequenceGen_(sequenceGen), discountTS_(discountTS) {
        QL_REQUIRE(underlying>0.0,
                   "underlying less/equal zero not allowed");
    }

    Real DigitalPathPricer::operator()(const Path& path) const {
        Size i, n = path.size();
        QL_REQUIRE(n>0, "the path cannot be empty");

        Real log_asset_price = QL_LOG(underlying_);
        Real x, y;
        Volatility vol;
        TimeGrid timeGrid = path.timeGrid();
        Time dt;
        Array u = sequenceGen_.nextSequence().value;
        Real log_strike = QL_LOG(payoff_->strike());

        switch (payoff_->optionType()) {
          case Option::Call:
            for (i=0; i<n; i++) {
                x = path.drift()[i] + path.diffusion()[i];
                // terminal or initial vol?
                // initial (timeGrid[i+1]) for the time being
                vol = diffProcess_->diffusion(timeGrid[i+1],
                                              QL_EXP(log_asset_price));
                // vol = diffProcess_->diffusion(timeGrid[i+2],
                //                               QL_EXP(log_asset_price+x));
                dt = timeGrid.dt(i);
                y = log_asset_price +
                    0.5*(x + QL_SQRT(x*x-2*vol*vol*dt*QL_LOG((1-u[i]))));
                // cross the strike
                if (y >= log_strike) {
                    if (exercise_->payoffAtExpiry()) {
                        return payoff_->cashPayoff() *
                            discountTS_->discount(path.timeGrid().back());
                    } else {
                        // the discount should be calculated at the exercise
                        // time between path.timeGrid()[i+1] and
                        // path.timeGrid()[i+2]
                        return payoff_->cashPayoff() *
                            discountTS_->discount(path.timeGrid()[i+1]);
                    }
                }
                log_asset_price += x;
            }
            break;
          case Option::Put:
            for (i = 0; i < n; i++) {
                x = path.drift()[i]+path.diffusion()[i];
                // terminal or initial vol?
                // initial (timeGrid[i+1]) for the time being
                vol = diffProcess_->diffusion(timeGrid[i+1],
                                              QL_EXP(log_asset_price));
                // vol = diffProcess_->diffusion(timeGrid[i+2],
                //                               QL_EXP(log_asset_price+x));
                dt = timeGrid.dt(i);
                y = log_asset_price +
                    0.5*(x - QL_SQRT(x*x - 2*vol*vol*dt*QL_LOG(u[i])));
                if (y <= log_strike) {
                    if (exercise_->payoffAtExpiry()) {
                        return payoff_->cashPayoff() *
                            discountTS_->discount(path.timeGrid().back());
                    } else {
                        // the discount should be calculated at the exercise
                        // time between path.timeGrid()[i+1] and
                        // path.timeGrid()[i+2]
                        return payoff_->cashPayoff() *
                            discountTS_->discount(path.timeGrid()[i+1]);
                    }
                }
                log_asset_price += x;
            }
            break;
          default:
            QL_FAIL("unknown option type");
        }

        return 0.0;
    }

}

