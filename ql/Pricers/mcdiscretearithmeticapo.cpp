
/*
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

#include <ql/Pricers/mcdiscretearithmeticapo.hpp>
#include <ql/Pricers/discretegeometricapo.hpp>
#include <ql/TermStructures/flatforward.hpp>

namespace QuantLib {

    namespace {

        class ArithmeticAPOPathPricer : public PathPricer<Path> {
          public:
            ArithmeticAPOPathPricer(Option::Type type,
                                    double underlying,
                                    double strike,
                                    const RelinkableHandle<TermStructure>&
                                                                   discountTS)
            : PathPricer<Path>(discountTS),
              underlying_(underlying), payoff_(type, strike) {
                QL_REQUIRE(underlying>0.0,
                           "ArithmeticAPOPathPricer: "
                           "underlying less/equal zero not allowed");
                QL_REQUIRE(strike>=0.0,
                           "ArithmeticAPOPathPricer: "
                           "strike less than zero not allowed");
            }

            double operator()(const Path& path) const  {

                Size n = path.size();
                QL_REQUIRE(n>0,
                           "ArithmeticAPOPathPricer: "
                           "the path cannot be empty");

                double price1 = underlying_;
                double averagePrice1 = 0.0;
                Size fixings = n;
                if (path.timeGrid().mandatoryTimes()[0]==0.0) {
                    averagePrice1 = price1;
                    fixings = n+1;
                }
                Size i;
                for (i=0; i<n; i++) {
                    price1 *= QL_EXP(path[i]);
                    averagePrice1 += price1;
                }
                averagePrice1 = averagePrice1/fixings;

                return discountTS_->discount(path.timeGrid().back())
                    * payoff_(averagePrice1);
            }

          private:
            double underlying_;
            PlainVanillaPayoff payoff_;
        };

        class GeometricAPOPathPricer : public PathPricer<Path> {
          public:
            GeometricAPOPathPricer(Option::Type type,
                                   double underlying,
                                   double strike,
                                   const RelinkableHandle<TermStructure>&
                                                                   discountTS)
            : PathPricer<Path>(discountTS),
              underlying_(underlying), payoff_(type, strike) {
                QL_REQUIRE(underlying>0.0,
                           "GeometricAPOPathPricer: "
                           "underlying less/equal zero not allowed");
                QL_REQUIRE(strike>=0.0,
                           "GeometricAPOPathPricer: "
                           "strike less than zero not allowed");
            }

            double operator()(const Path& path) const {
                Size n = path.size();
                QL_REQUIRE(n>0,
                           "GeometricAPOPathPricer: "
                           "the path cannot be empty");

                double geoLogVariation = 0.0;
                Size i;
                for (i=0; i<n; i++)
                    geoLogVariation += (n-i)*path[i];
                Size fixings = n;
                if (path.timeGrid().mandatoryTimes()[0]==0.0)
                    fixings = n+1;
                double averagePrice1 = underlying_*
                    QL_EXP(geoLogVariation/fixings);

                return discountTS_->discount(path.timeGrid().back())
                    * payoff_(averagePrice1);
            }

          private:
            double underlying_;
            PlainVanillaPayoff payoff_;
        };

    }


    McDiscreteArithmeticAPO::McDiscreteArithmeticAPO(Option::Type type,
        double underlying, double strike,
        Spread dividendYield, Rate riskFreeRate,
        const std::vector<Time>& times, double volatility,
        bool controlVariate, long seed) {

        QL_REQUIRE(times.size() >= 2,
                   "McDiscreteArithmeticAPO: "
                   "you must have at least 2 time-steps");

        // initialize the path generator
        double mu = riskFreeRate - dividendYield
                                 - 0.5 * volatility * volatility;

        boost::shared_ptr<GaussianPathGenerator_old> pathGenerator(
            new GaussianPathGenerator_old(mu, volatility*volatility,
                TimeGrid(times.begin(), times.end()),
                seed));

        RelinkableHandle<TermStructure> discount(
                  Handle<TermStructure>(
                      new FlatForward(Date::todaysDate(), Date::todaysDate(), 
                                      riskFreeRate)));

        // initialize the Path Pricer
        boost::shared_ptr<PathPricer<Path> > spPricer(
             new ArithmeticAPOPathPricer(type, underlying, strike, discount));

        if (controlVariate) {
            boost::shared_ptr<PathPricer<Path> > controlVariateSpPricer(
              new GeometricAPOPathPricer(type, underlying, strike, discount));

            double controlVariatePrice = DiscreteGeometricAPO(type,
                underlying, strike, dividendYield, riskFreeRate,
                times, volatility).value();

            // initialize the Monte Carlo model
            mcModel_ = boost::shared_ptr<MonteCarloModel<SingleAsset_old<
                                              PseudoRandom_old> > >(
                new MonteCarloModel<SingleAsset_old<PseudoRandom_old> >(
                    pathGenerator, spPricer, Statistics(), false,
                    controlVariateSpPricer, controlVariatePrice));
        } else {
            // initialize the Monte Carlo model
            mcModel_ = boost::shared_ptr<MonteCarloModel<SingleAsset_old<
                                              PseudoRandom_old> > > (
                new MonteCarloModel<SingleAsset_old<PseudoRandom_old> >(
                    pathGenerator, spPricer, Statistics(), false));
        }

    }

}
