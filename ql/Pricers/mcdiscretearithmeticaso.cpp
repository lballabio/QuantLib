/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
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

#include <ql/Pricers/mcdiscretearithmeticaso.hpp>
#include <ql/Pricers/discretegeometricaso.hpp>
#include <ql/Processes/blackscholesprocess.hpp>

namespace QuantLib {

    namespace {

        class ArithmeticASOPathPricer : public PathPricer<Path> {
          public:
            ArithmeticASOPathPricer(Option::Type type,
                                    Real underlying,
                                    DiscountFactor discount)
            : type_(type), underlying_(underlying), discount_(discount) {
                QL_REQUIRE(underlying>0.0,
                           "underlying less/equal zero not allowed");
            }

            Real operator()(const Path& path) const  {
                Size n = path.length()-1;
                QL_REQUIRE(n > 0, "the path cannot be empty");

                Real price1 = underlying_;
                Real averageStrike1 = 0.0;
                Size fixings = n;
                if (path.timeGrid().mandatoryTimes()[0]==0.0) {
                    averageStrike1 = price1;
                    fixings = n+1;
                }
                for (Size i=0; i<n; i++) {
                    price1 = path.value(i+1);
                    averageStrike1 += price1;
                }
                averageStrike1 = averageStrike1/fixings;

                return discount_
                    * PlainVanillaPayoff(type_, averageStrike1)(price1);
            }

          private:
            Option::Type type_;
            Real underlying_;
            DiscountFactor discount_;
        };

        class GeometricASOPathPricer : public PathPricer<Path> {
          public:
            GeometricASOPathPricer(Option::Type type,
                                   Real underlying,
                                   DiscountFactor discount)
            : type_(type), underlying_(underlying), discount_(discount) {
                QL_REQUIRE(underlying>0.0,
                           "underlying less/equal zero not allowed");
            }

            Real operator()(const Path& path) const {

                Size n = path.length()-1;
                QL_REQUIRE(n>0, "the path cannot be empty");

                Size fixings = n;
                if (path.timeGrid().mandatoryTimes()[0]==0.0)
                    fixings = n+1;

                Real averageStrike = 1.0;
                Real maxValue = QL_MAX_REAL;
                Real product = 1.0;
                if (path.timeGrid().mandatoryTimes()[0]==0.0)
                    product = underlying_;
                for (Size i=0; i<n; i++) {
                    Real value = path.value(i+1);
                    if (product < maxValue/value) {
                        product *= value;
                    } else {
                        averageStrike *= std::pow(product, 1.0/fixings);
                        product = value;
                    }
                }
                averageStrike *= std::pow(product, 1.0/fixings);
                return discount_
                    * PlainVanillaPayoff(type_, averageStrike)(path.back());
            }
          private:
            Option::Type type_;
            Real underlying_;
            DiscountFactor discount_;
        };

    }

    McDiscreteArithmeticASO::McDiscreteArithmeticASO(
                              Option::Type type,
                              Real underlying,
                              const Handle<YieldTermStructure>& dividendYield,
                              const Handle<YieldTermStructure>& riskFreeRate,
                              const Handle<BlackVolTermStructure>& volatility,
                              const std::vector<Time>& times,
                              bool controlVariate,
                              BigNatural seed) {

        QL_REQUIRE(times.size() >= 2,
                   "you must have at least 2 time-steps");

        // initialize the path generator
        Handle<Quote> u(boost::shared_ptr<Quote>(new SimpleQuote(underlying)));
        boost::shared_ptr<StochasticProcess1D> diffusion(
                                     new BlackScholesProcess(u,
                                                             dividendYield,
                                                             riskFreeRate,
                                                             volatility));
        TimeGrid grid(times.begin(), times.end());
        PseudoRandom::rsg_type rsg =
            PseudoRandom::make_sequence_generator(grid.size()-1,seed);

        bool brownianBridge = false;

        typedef SingleAsset<PseudoRandom>::path_generator_type generator;
        boost::shared_ptr<generator> pathGenerator(new
            generator(diffusion, grid, rsg, brownianBridge));

        // initialize the path pricer
        DiscountFactor discount = riskFreeRate->discount(times.back());
        boost::shared_ptr<PathPricer<Path> > spPricer(
                     new ArithmeticASOPathPricer(type, underlying, discount));

        if (controlVariate) {
            boost::shared_ptr<PathPricer<Path> > controlVariateSpPricer(
                     new GeometricASOPathPricer(type, underlying, discount));

            // Not sure whether this work when curves are not flat...
            Time exercise = times.back();
            Rate r = riskFreeRate->zeroRate(exercise, Continuous, NoFrequency);
            Rate q = dividendYield->zeroRate(exercise, Continuous, NoFrequency);
            Volatility sigma = volatility->blackVol(exercise,underlying);

            Real controlVariatePrice = DiscreteGeometricASO(type,
                underlying, q, r, times, sigma).value();

            // initialize the one-dimensional Monte Carlo
            mcModel_ = boost::shared_ptr<MonteCarloModel<SingleAsset<
                                              PseudoRandom> > > (
                new MonteCarloModel<SingleAsset<PseudoRandom> >(
                    pathGenerator, spPricer, Statistics(), false,
                    controlVariateSpPricer, controlVariatePrice));
        } else {
            // initialize the one-dimensional Monte Carlo
            mcModel_ = boost::shared_ptr<MonteCarloModel<SingleAsset<
                                              PseudoRandom> > > (
                new MonteCarloModel<SingleAsset<PseudoRandom> >(
                    pathGenerator, spPricer, Statistics(), false));
        }

    }

}
