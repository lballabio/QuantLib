
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

#include <ql/Pricers/mcdiscretearithmeticaso.hpp>
#include <ql/Pricers/discretegeometricaso.hpp>
#include <ql/TermStructures/flatforward.hpp>

namespace QuantLib {

    namespace {

        class ArithmeticASOPathPricer : public PathPricer<Path> {
          public:
            ArithmeticASOPathPricer(Option::Type type,
                                    double underlying,
                                    const RelinkableHandle<TermStructure>&
                                                                   discountTS)
            : PathPricer<Path>(discountTS), 
              type_(type), underlying_(underlying) {
                QL_REQUIRE(underlying>0.0,
                           "ArithmeticASOPathPricer: "
                           "underlying less/equal zero not allowed");
            }

            double operator()(const Path& path) const  {

                Size n = path.size();
                QL_REQUIRE(n > 0,
                           "ArithmeticASOPathPricer: "
                           "the path cannot be empty");

                double price1 = underlying_;
                double averageStrike1 = 0.0;
                Size fixings = n;
                if (path.timeGrid().mandatoryTimes()[0]==0.0) {
                    averageStrike1 = price1;
                    fixings = n+1;
                }
                Size i;
                for (i=0; i<n; i++) {
                    price1 *= QL_EXP(path[i]);
                    averageStrike1 += price1;
                }
                averageStrike1 = averageStrike1/fixings;

                return discountTS_->discount(path.timeGrid().back()) 
                    * PlainVanillaPayoff(type_, averageStrike1)(price1);
            }

          private:
            Option::Type type_;
            double underlying_;
        };

        class GeometricASOPathPricer : public PathPricer<Path> {
          public:
            GeometricASOPathPricer(Option::Type type,
                                   double underlying,
                                   const RelinkableHandle<TermStructure>&
                                                                   discountTS)
            : PathPricer<Path>(discountTS), 
              type_(type), underlying_(underlying) {
                QL_REQUIRE(underlying>0.0,
                           "GeometricASOPathPricer: "
                           "underlying less/equal zero not allowed");
            }

            double operator()(const Path& path) const {

                Size n = path.size();
                QL_REQUIRE(n>0,
                           "GeometricASOPathPricer: "
                           "the path cannot be empty");

                double logVariation = 0.0;
                double geoLogVariation = 0.0;
                Size i;
                for (i=0; i<n; i++) {
                    logVariation += path[i];
                    geoLogVariation += (n-i)*path[i];
                }
                Size fixings = n;
                if (path.timeGrid().mandatoryTimes()[0]==0.0) {
                    fixings = n+1;
                }
                double averageStrike1 = underlying_*
                    QL_EXP(geoLogVariation/fixings);

                return discountTS_->discount(path.timeGrid().back())
                    * PlainVanillaPayoff(type_, averageStrike1)
                    (underlying_ * QL_EXP(logVariation));
            }

          private:
            Option::Type type_;
            double underlying_;
        };

    }

    McDiscreteArithmeticASO::McDiscreteArithmeticASO(Option::Type type,
      double underlying,
      Spread dividendYield, Rate riskFreeRate,
      const std::vector<Time>& times, double volatility,
      bool controlVariate, long seed) {

        QL_REQUIRE(times.size() >= 2,
                   "McDiscreteArithmeticASO: "
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
                     new ArithmeticASOPathPricer(type, underlying, discount));

        if (controlVariate) {
            boost::shared_ptr<PathPricer<Path> > controlVariateSpPricer(
                      new GeometricASOPathPricer(type, underlying, discount));

            double controlVariatePrice = DiscreteGeometricASO(type,
                underlying, dividendYield, riskFreeRate,
                times, volatility).value();

            // initialize the one-dimensional Monte Carlo
            mcModel_ = boost::shared_ptr<MonteCarloModel<SingleAsset_old<
                                              PseudoRandom_old> > > (
                new MonteCarloModel<SingleAsset_old<PseudoRandom_old> >(
                    pathGenerator, spPricer, Statistics(), false,
                    controlVariateSpPricer, controlVariatePrice));
        } else {
            // initialize the one-dimensional Monte Carlo
            mcModel_ = boost::shared_ptr<MonteCarloModel<SingleAsset_old<
                                              PseudoRandom_old> > > (
                new MonteCarloModel<SingleAsset_old<PseudoRandom_old> >(
                    pathGenerator, spPricer, Statistics(), false));
        }

    }

}
