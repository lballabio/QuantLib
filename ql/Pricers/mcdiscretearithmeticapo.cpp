
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
                           "underlying less/equal zero not allowed");
                QL_REQUIRE(strike>=0.0,
                           "strike less than zero not allowed");
            }

            double operator()(const Path& path) const  {

                Size n = path.size();
                QL_REQUIRE(n>0,
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
                           "underlying less/equal zero not allowed");
                QL_REQUIRE(strike>=0.0,
                           "strike less than zero not allowed");
            }

            double operator()(const Path& path) const {
                Size n = path.size();
                QL_REQUIRE(n>0,
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


    McDiscreteArithmeticAPO::McDiscreteArithmeticAPO(
                    Option::Type type,
                    double underlying,
                    double strike,
                    const RelinkableHandle<TermStructure>& dividendYield,
                    const RelinkableHandle<TermStructure>& riskFreeRate,
                    const RelinkableHandle<BlackVolTermStructure>& volatility,
                    const std::vector<Time>& times,
                    bool controlVariate,
                    long seed) {

        QL_REQUIRE(times.size() >= 2,
                   "you must have at least 2 time-steps");

        // initialize the path generator
        RelinkableHandle<Quote> u(
                    boost::shared_ptr<Quote>(new SimpleQuote(underlying)));
        boost::shared_ptr<DiffusionProcess> diffusion(
                                     new BlackScholesProcess(u, 
                                                             dividendYield,
                                                             riskFreeRate, 
                                                             volatility));
        TimeGrid grid(times.begin(), times.end());
        PseudoRandom::rsg_type rsg =
            PseudoRandom::make_sequence_generator(grid.size()-1,seed);

        typedef SingleAsset<PseudoRandom>::path_generator_type generator;
        boost::shared_ptr<generator> pathGenerator(
                                  new generator(diffusion, grid, rsg, false));

        // initialize the Path Pricer
        boost::shared_ptr<PathPricer<Path> > spPricer(
                           new ArithmeticAPOPathPricer(type, underlying, 
                                                       strike, riskFreeRate));

        if (controlVariate) {
            boost::shared_ptr<PathPricer<Path> > controlVariateSpPricer(
                            new GeometricAPOPathPricer(type, underlying, 
                                                       strike, riskFreeRate));

            // Not sure whether this work when curves are not flat...
            Time exercise = times.back();
            Rate r = riskFreeRate->zeroYield(exercise);
            Rate q = dividendYield->zeroYield(exercise);
            double sigma = volatility->blackVol(exercise,underlying);

            double controlVariatePrice = DiscreteGeometricAPO(type,
                underlying, strike, q, r, times, sigma).value();

            // initialize the Monte Carlo model
            mcModel_ = boost::shared_ptr<MonteCarloModel<SingleAsset<
                                              PseudoRandom> > >(
                new MonteCarloModel<SingleAsset<PseudoRandom> >(
                    pathGenerator, spPricer, Statistics(), false,
                    controlVariateSpPricer, controlVariatePrice));
        } else {
            // initialize the Monte Carlo model
            mcModel_ = boost::shared_ptr<MonteCarloModel<SingleAsset<
                                              PseudoRandom> > > (
                new MonteCarloModel<SingleAsset<PseudoRandom> >(
                    pathGenerator, spPricer, Statistics(), false));
        }

    }

}
