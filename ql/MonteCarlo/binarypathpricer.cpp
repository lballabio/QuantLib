
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

/*! \file binarypathpricer.cpp
    \brief path pricer for Binary options
*/

#include <ql/MonteCarlo/binarypathpricer.hpp>

namespace QuantLib {

    using namespace RandomNumbers;

    BinaryPathPricer::BinaryPathPricer(
                            Binary::Type binaryType, 
                            double barrier, 
                            double cashPayoff, 
                            Option::Type type,
                            double underlying,
                            const RelinkableHandle<TermStructure>& riskFreeTS,
                            const Handle<DiffusionProcess>& diffProcess,
                            UniformRandomSequenceGenerator sequenceGen)
    : PathPricer<Path>(riskFreeTS), 
      binaryType_(binaryType),
      barrier_(barrier), cashPayoff_(cashPayoff), 
      type_(type), underlying_(underlying),
      diffProcess_(diffProcess), sequenceGen_(sequenceGen) {
        QL_REQUIRE(underlying>0.0,
                   "BinaryPathPricer: "
                   "underlying less/equal zero not allowed");
        QL_REQUIRE(barrier>0.0,
                   "BinaryPathPricer: "
                   "barrier less/equal zero not allowed");
    }

    double BinaryPathPricer::operator()(const Path& path) const {
        Size i, n = path.size();
        QL_REQUIRE(n>0,
                   "BinaryPathPricer: the path cannot be empty");

        double asset_price = underlying_;
        double new_asset_price;
        double x, y;
        double vol;
        TimeGrid timeGrid = path.timeGrid();
        Time dt;
        double log_drift, log_random;
        Array u = sequenceGen_.nextSequence().value;

        switch (type_) {
          case Option::Call:
            for (i = 0; i < n; i++) {
                log_drift = path.drift()[i];
                log_random = path.diffusion()[i];
                new_asset_price = 
                    asset_price * QL_EXP(log_drift+log_random);
                // terminal or initial vol?                        
                vol = diffProcess_->diffusion(timeGrid.at(i),asset_price);
                dt = timeGrid.dt(i);

                x = QL_LOG (new_asset_price / asset_price);
                y = 0.5*(x + QL_SQRT (x*x-2*vol*vol*dt*QL_LOG((1-u[i]))));
                y = asset_price * QL_EXP (y);
                // cross the barrier
                if (y >= barrier_) {
                    if (binaryType_ == Binary::CashAtExpiry) {
                        return cashPayoff_ * 
                            riskFreeTS_->discount(path.timeGrid().back());

                    } else if (binaryType_ == Binary::CashAtHit) {
                        return cashPayoff_ * 
                            riskFreeTS_->discount(path.timeGrid()[i]);

                    } else {
                        throw Error("BinaryPathPricer: "
                                    "unknown binary type");
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
                vol = diffProcess_->diffusion(timeGrid.at(i),asset_price);
                dt = timeGrid.dt(i);

                x = QL_LOG (new_asset_price / asset_price);
                y = 0.5*(x - QL_SQRT (x*x - 2*vol*vol*dt*QL_LOG(u[i])));
                y = asset_price * QL_EXP (y);
                if (y <= barrier_) {
                    if (binaryType_ == Binary::CashAtExpiry) {
                        return cashPayoff_ * 
                            riskFreeTS_->discount(path.timeGrid().back());

                    } else if (binaryType_ == Binary::CashAtHit) {
                        return cashPayoff_ 
                            * riskFreeTS_->discount(path.timeGrid()[i]);

                    } else {
                        throw Error("BinaryPathPricer: "
                                    "unknown binary type");
                    }
                }
                asset_price = new_asset_price;
            }
            break;
          default:
            throw Error("BinaryPathPricer: unknown option type");
        }

        return 0.0;
    }

}

