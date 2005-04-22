/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 Neil Firth
 Copyright (C) 2003 Ferdinando Ametrano
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

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

#include <ql/PricingEngines/Barrier/mcbarrierengine.hpp>

namespace QuantLib {

    BarrierPathPricer::BarrierPathPricer(
                    Barrier::Type barrierType,
                    Real barrier,
                    Real rebate,
                    Option::Type type,
                    Real underlying,
                    Real strike,
                    DiscountFactor discount,
                    const boost::shared_ptr<StochasticProcess1D>& diffProcess,
                    const PseudoRandom::ursg_type& sequenceGen)
    : underlying_(underlying),
      barrierType_(barrierType), barrier_(barrier),
      rebate_(rebate), diffProcess_(diffProcess),
      sequenceGen_(sequenceGen), payoff_(type, strike),
      discount_(discount) {
        QL_REQUIRE(underlying>0.0,
                   "underlying less/equal zero not allowed");
        QL_REQUIRE(strike>=0.0,
                   "strike less than zero not allowed");
        QL_REQUIRE(barrier>0.0,
                   "barrier less/equal zero not allowed");
    }


    Real BarrierPathPricer::operator()(const Path& path) const {
        Size n = path.size();
        QL_REQUIRE(n>0, "the path cannot be empty");

        bool isOptionActive = false;
        Real asset_price = underlying_;
        Real new_asset_price;
        Real x, y;
        Volatility vol;
        TimeGrid timeGrid = path.timeGrid();
        Time dt;
        Real log_drift, log_random;
        Array u = sequenceGen_.nextSequence().value;
        Size i;

        switch (barrierType_) {
          case Barrier::DownIn:
            isOptionActive = false;
            for (i = 0; i < n; i++) {
                log_drift = path.drift()[i];
                log_random = path.diffusion()[i];
                new_asset_price = asset_price * std::exp(log_drift+log_random);
                // terminal or initial vol?
                vol = diffProcess_->diffusion(timeGrid[i],asset_price);
                dt = timeGrid.dt(i);

                x = std::log(new_asset_price / asset_price);
                y = 0.5*(x - std::sqrt (x*x - 2*vol*vol*dt*std::log(u[i])));
                y = asset_price * std::exp(y);
                if (y <= barrier_) {
                    isOptionActive = true;
                }
                asset_price = new_asset_price;
            }
            break;
          case Barrier::UpIn:
            isOptionActive = false;
            for (i = 0; i < n; i++) {
                log_drift = path.drift()[i];
                log_random = path.diffusion()[i];
                new_asset_price = asset_price * std::exp(log_drift+log_random);
                // terminal or initial vol?
                vol = diffProcess_->diffusion(timeGrid[i],asset_price);
                dt = timeGrid.dt(i);

                x = std::log(new_asset_price / asset_price);
                y = 0.5*(x + std::sqrt(x*x - 2*vol*vol*dt*std::log((1-u[i]))));
                y = asset_price * std::exp(y);
                if (y >= barrier_) {
                    isOptionActive = true;
                }
                asset_price = new_asset_price;
            }
            break;
          case Barrier::DownOut:
            isOptionActive = true;
            for (i = 0; i < n; i++) {
                log_drift = path.drift()[i];
                log_random = path.diffusion()[i];
                new_asset_price = asset_price * std::exp(log_drift+log_random);
                // terminal or initial vol?
                vol = diffProcess_->diffusion(timeGrid[i],asset_price);
                dt = timeGrid.dt(i);

                x = std::log(new_asset_price / asset_price);
                y = 0.5*(x - std::sqrt(x*x - 2*vol*vol*dt*std::log(u[i])));
                y = asset_price * std::exp(y);
                if (y <= barrier_) {
                    isOptionActive = false;
                }
                asset_price = new_asset_price;
            }
            break;
          case Barrier::UpOut:
            isOptionActive = true;
            for (i = 0; i < n; i++) {
                log_drift = path.drift()[i];
                log_random = path.diffusion()[i];
                new_asset_price = asset_price * std::exp(log_drift+log_random);
                // terminal or initial vol?
                vol = diffProcess_->diffusion(timeGrid[i],asset_price);
                dt = timeGrid.dt(i);

                x = std::log(new_asset_price / asset_price);
                y = 0.5*(x + std::sqrt(x*x - 2*vol*vol*dt*std::log((1-u[i]))));
                y = asset_price * std::exp(y);
                if (y >= barrier_) {
                    isOptionActive = false;
                }
                asset_price = new_asset_price;
            }
            break;
          default:
            QL_FAIL("unknown barrier type");
        }

        if (isOptionActive) {
            return payoff_(asset_price) * discount_;
        } else {
            return 0.0;
        }
    }


    BiasedBarrierPathPricer::BiasedBarrierPathPricer(
                                      Barrier::Type barrierType,
                                      Real barrier,
                                      Real rebate,
                                      Option::Type type,
                                      Real underlying,
                                      Real strike,
                                      DiscountFactor discount)
    : underlying_(underlying),
      barrierType_(barrierType), barrier_(barrier),
      rebate_(rebate), payoff_(type, strike), discount_(discount) {
        QL_REQUIRE(underlying>0.0,
                   "underlying less/equal zero not allowed");
        QL_REQUIRE(strike>=0.0,
                   "strike less than zero not allowed");
        QL_REQUIRE(barrier>0.0,
                   "barrier less/equal zero not allowed");
    }


    Real BiasedBarrierPathPricer::operator()(const Path& path) const {
        Size n = path.size();
        QL_REQUIRE(n>0, "the path cannot be empty");

        bool isOptionActive = false;
        Real asset_price = underlying_;
        Real log_drift, log_random;
        Size i;

        switch (barrierType_) {
          case Barrier::DownIn:
            isOptionActive = false;
            for (i = 0; i < n; i++) {
                log_drift = path.drift()[i];
                log_random = path.diffusion()[i];
                asset_price = asset_price * std::exp(log_drift+log_random);
                if (asset_price <= barrier_) {
                    isOptionActive = true;
                }
            }
            break;
          case Barrier::UpIn:
            isOptionActive = false;
            for (i = 0; i < n; i++) {
                log_drift = path.drift()[i];
                log_random = path.diffusion()[i];
                asset_price = asset_price * std::exp(log_drift+log_random);
                if (asset_price >= barrier_) {
                    isOptionActive = true;
                }
            }
            break;
          case Barrier::DownOut:
            isOptionActive = true;
            for (i = 0; i < n; i++) {
                log_drift = path.drift()[i];
                log_random = path.diffusion()[i];
                asset_price = asset_price * std::exp(log_drift+log_random);
                if (asset_price <= barrier_) {
                    isOptionActive = false;
                }
            }
            break;
          case Barrier::UpOut:
            isOptionActive = true;
            for (i = 0; i < n; i++) {
                log_drift = path.drift()[i];
                log_random = path.diffusion()[i];
                asset_price = asset_price * std::exp(log_drift+log_random);
                if (asset_price >= barrier_) {
                    isOptionActive = false;
                }
            }
            break;
          default:
            QL_FAIL("unknown barrier type");
        }

        if (isOptionActive) {
            return payoff_(asset_price) * discount_;
        } else {
            return 0.0;
        }
    }

}

