
/*
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

/*! \file binarybarrierpathpricer.cpp
    \brief path pricer for binary barrier options
*/

#include <ql/MonteCarlo/binarybarrierpathpricer.hpp>

namespace QuantLib {

    BinaryBarrierPathPricer::BinaryBarrierPathPricer(
        const Handle<CashOrNothingPayoff>& payoff,
        const Handle<AmericanExercise>& exercise,
        double underlying,
        const RelinkableHandle<TermStructure>& riskFreeTS,
        const Handle<DiffusionProcess>& diffProcess,
        UniformRandomSequenceGenerator sequenceGen)
    : PathPricer<Path>(riskFreeTS), 
      payoff_(payoff), exercise_(exercise),
      underlying_(underlying),
      diffProcess_(diffProcess), sequenceGen_(sequenceGen) {
        QL_REQUIRE(underlying>0.0,
                   "BinaryBarrierPathPricer: "
                   "underlying less/equal zero not allowed");
    }

    double BinaryBarrierPathPricer::operator()(const Path& path) const {
        Size i, n = path.size();
        QL_REQUIRE(n>0,
                   "BinaryBarrierPathPricer: the path cannot be empty");

        double asset_price = underlying_;
        double new_asset_price;
        double x, y;
        double vol;
        TimeGrid timeGrid = path.timeGrid();
        Time dt;
        double log_drift, log_random;
        Array u = sequenceGen_.nextSequence().value;

        switch (payoff_->optionType()) {
          case Option::Call:
            for (i = 0; i < n; i++) {
                log_drift = path.drift()[i];
                log_random = path.diffusion()[i];
                new_asset_price = 
                    asset_price * QL_EXP(log_drift+log_random);
                // terminal or initial vol?                        
                vol = diffProcess_->diffusion(timeGrid[i],asset_price);
                dt = timeGrid.dt(i);

                x = QL_LOG (new_asset_price / asset_price);
                y = 0.5*(x + QL_SQRT (x*x-2*vol*vol*dt*QL_LOG((1-u[i]))));
                y = asset_price * QL_EXP (y);
                // cross the barrier
                if (y >= payoff_->strike()) {
                    if (exercise_->payoffAtExpiry()) {
                        return payoff_->cashPayoff() * 
                            riskFreeTS_->discount(path.timeGrid().back());

                    } else {
                        return payoff_->cashPayoff() * 
                            riskFreeTS_->discount(path.timeGrid()[i+1]);
                    }
                }
                asset_price = new_asset_price;
            }
            break;
          case Option::Put:
            for (i = 0; i < n; i++) {
                log_drift = path.drift()[i];
                log_random = path.diffusion()[i];
                new_asset_price = 
                    asset_price * QL_EXP(log_drift+log_random);
                // terminal or initial vol?                        
                vol = diffProcess_->diffusion(timeGrid[i],asset_price);
                dt = timeGrid.dt(i);

                x = QL_LOG (new_asset_price / asset_price);
                y = 0.5*(x - QL_SQRT (x*x - 2*vol*vol*dt*QL_LOG(u[i])));
                y = asset_price * QL_EXP (y);
                if (y <= payoff_->strike()) {
                    if (exercise_->payoffAtExpiry()) {
                        return payoff_->cashPayoff() * 
                            riskFreeTS_->discount(path.timeGrid().back());

                    } else {
                        return payoff_->cashPayoff() 
                            * riskFreeTS_->discount(path.timeGrid()[i+1]);
                    }
                }
                asset_price = new_asset_price;
            }
            break;
          default:
            QL_FAIL("BinaryBarrierPathPricer: unknown option type");
        }

        return 0.0;
    }

}

